import ray
import torch
import torch.nn as nn
import torch.optim as optim
from typing import List
from model import SimpleMLP
from data import make_train_loader

from utils import StateDict
from utils import state_dict_to_cpu

@ray.remote(num_cpus=0.5)
class FederatedWorker:

	#costruttore del worker
	def __init__(self, worker_id: int, data_dir: str, indices: List[int],
			batch_size: int, learning_rate: float, device: str) -> None:

		#identificativo de worker
		self.worker_id = worker_id

		#learning rate
		self.learning_rate = learning_rate

		#il device, cpu o cuda
		self.device = torch.device(device)

		#DataLoader locale usando solo indici assegnati al worker
		self.train_loader = make_train_loader(data_dir=data_dir, indices=indices, batch_size=batch_size)

		#numero di esempi locali del worker
		self.num_samples = len(indices)


		#funzione di loss
		self.criterion = nn.CrossEntropyLoss()



	#esegue un round locale di training
	def train_one_round(self, global_state_dict: StateDict, local_epochs: int):

		#nuova istanza del modello
		model = SimpleMLP().to(self.device)

		#i pesi globali ricevuti dall'aggregatore vengono caricati nel modello
		model.load_state_dict(global_state_dict)

		#modello in modalità training
		model.train()

		#ottimizzatore Adam per aggiornare i pesi locali
		optimizer = optim.Adam(model.parameters(), lr=self.learning_rate)

		total_loss = 0.0
		num_batches = 0

		for _ in range(local_epochs):

			for data, target in self.train_loader:

				#immagini e label vengono spostati sul device scelto
				data = data.to(self.device)
				target = target.to(self.device)

				optimizer.zero_grad()
				output = model(data)

				#calcolo loss confrontando output e label
				loss = self.criterion(output, target)

				#calcolo gradienti
				loss.backward()

				#aggiornamento pesi del modello locale
				optimizer.step()

				#somma della loss del batch
				total_loss += loss.item()

				num_batches += 1

		#calcolo loss media locale
		average_loss = total_loss / max(num_batches, 1)

		local_state_dict = state_dict_to_cpu(model.state_dict())

		return local_state_dict, self.num_samples, average_loss


	#controlla quanti dati ha il worker
	def get_num_samples(self) -> int:
		return self.num_samples
















