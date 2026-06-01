import argparse
import ray

from data import download_mnist
from data import get_train_dataset_size
from data import split_dataset_indices

from worker import FederatedWorker

from aggregator import Aggregator

from utils import mean_worker_loss


def parse_args():

	parser = argparse.ArgumentParser(description="Federated Learning con Ray.io su MNIST")

	parser.add_argument("--data-dir", type=str, default="./data", help="Cartella dove scaricare/caricare MNIST")

	parser.add_argument("--num-workers", type=int, default=6, help="Numero di worker federati")

	parser.add_argument("--rounds", type=int, default=5, help="Numero di round federati")

	parser.add_argument("--local-epochs", type=int, default=1, help="Numero di epoche locali per worker in ogni round")

	parser.add_argument("--batch-size", type=int, default=64, help="Batch size per il training locale")

	parser.add_argument("--test-batch-size", type=int, default=1000, help="Batch size per il test")

	parser.add_argument("--learning-rate", type=float, default=0.001, help="Learning rate per Adam")

	parser.add_argument("--device", type=str, default="cpu", choices=["cpu", "cuda"], help="Device da usare: cpu o cuda")

	parser.add_argument("--seed", type=int, default=42, help="Seed per la divisione casuale del dataset")

	return parser.parse_args()


def main() -> None:

	args = parse_args()

	ray_temp_dir = "/tmp/ray/ho19"

	#Avvio di ray in locale
	ray.init(ignore_reinit_error=True, _temp_dir=ray_temp_dir)

	print("\n------Federated Learning MNIST con Ray.io------\n")

	print(f"Numero di worker: {args.num_workers}")

	print(f"Round federati: {args.rounds}")

	print(f"Epoche locali per round: {args.local_epochs}")

	print(f"Device: {args.device}")

	print("\nDownload/caricamento dataset MNIST ...")
	download_mnist(args.data_dir)

	#numero di esempi di training
	num_train_samples = get_train_dataset_size(args.data_dir)

	#indici del dataset divisi tra i worker
	worker_indices = split_dataset_indices(num_samples=num_train_samples, num_workers=args.num_workers, seed=args.seed)

	#aggregatore remoto
	aggregator = Aggregator.remote(data_dir=args.data_dir, test_batch_size=args.test_batch_size, device=args.device)

	workers = []

	for worker_id in range(args.num_workers):

		worker = FederatedWorker.remote(worker_id=worker_id, data_dir=args.data_dir, indices=worker_indices[worker_id],
						batch_size=args.batch_size, learning_rate=args.learning_rate, device=args.device)

		workers.append(worker)

	print("\nDistribuzione dei dati tra worker:")

	sample_counts = ray.get([worker.get_num_samples.remote() for worker in workers])

	for worker_id, count in enumerate(sample_counts):
		print(f"Worker {worker_id}: {count} esempi")

	#round federati
	print("\nInizio training federato ...")

	#ciclo round federati
	for round_id in range(1, args.rounds + 1):

		global_weights = ray.get(aggregator.get_global_weights.remote())

		#ogni worker allena localmente partendo dagli stessi pesi globali
		training_tasks = [worker.train_one_round.remote(global_state_dict=global_weights, local_epochs=args.local_epochs)
								for worker in workers]

		#si aspetta finchè tutti i worker finiscano il training locale
		worker_updates = ray.get(training_tasks)

		#l'aggregatore applica FedAvg agli aggiornamenti ricevuti
		ray.get(aggregator.aggregate.remote(worker_updates))

		#viene valutato il nuovo modello globale sul test set
		accuracy, test_loss = ray.get(aggregator.evaluate.remote())

		#loss media locale dei worker
		train_loss = mean_worker_loss(worker_updates)



		print(f"Round {round_id:02d} | "
			f"Train loss media worker: {train_loss:.4f} | "
			f"Test loss: {test_loss:.4f} | "
			f"Test accuracy: {accuracy * 100:.2f}%")

	#spegnimento di ray alla fine
	ray.shutdown()

	print("\nTraining federato completato.")

if __name__ == "__main__":
	main()
















































































