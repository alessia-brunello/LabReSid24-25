from pathlib import Path
from typing import List
import torch

from torchvision import datasets, transforms	#MNIST e trasformazioni per le immagini

from torch.utils.data import DataLoader		#per creare i batch di dati

from torch.utils.data import Subset		#per creare sottoinsiemi di dataset

#crea le trasformazioni da applicare a MNIST
def get_mnist_transform():

	#compose concatena più trasformazioni
	return transforms.Compose([
		transforms.ToTensor(), #immagine PIL --> tensore PyTorch

		transforms.Normalize((0.1307,), (0.3081,)) #normalizza i pixel

	])


#scarica MNIST se non è presente
def download_mnist(data_dir: str)->None:

	Path(data_dir).mkdir(parents=True, exist_ok=True)

	#trasformazioni standard per MNIST
	transform = get_mnist_transform()

	#scarica il training set MNIST
	datasets.MNIST(root=data_dir, train=True, download=True, transform=transform)

	#scarica il test set MNIST
	datasets.MNIST(root=data_dir, train=False, download=True, transform=transform)




#restituisce il numero di esempi nel training set
def get_train_dataset_size(data_dir: str)-> int:

	#carica il training set senza scaricarlo
	train_dataset = datasets.MNIST(root=data_dir, train=True, download=False, transform=get_mnist_transform())

	#lunghezza del dataset
	return len(train_dataset)


#divide gli indici del dataset tra i worker
def split_dataset_indices(num_samples: int, num_workers: int, seed: int = 42) -> List[List[int]]:

	#generatore casuale con seed fisso in modo da poter riprodurre la divisione dei dati
	generator = torch.Generator().manual_seed(seed)

	#permutazione casuale degli indici da 0 a num_samples - 1
	all_indices = torch.randperm(num_samples, generator=generator).tolist()

	#si calcola quanti elementi dare ad ogni worker
	split_size = num_samples // num_workers

	#lista che contiene una lista di indici per ongi worker
	splits = []

	for worker_id in range(num_workers):

		#indice iniziale
		start = worker_id * split_size

		#per l'ultimo worker si arriva alla fine
		if worker_id == num_workers - 1:
			end = num_samples

		#per gli altri worker usiamo blocchi di dimensione split_size
		else:
			end = start + split_size

		#si aggiungono gli indiciaì al worker
		splits.append(all_indices[start:end])


	return splits


#crea il DataLoader di training per un singolo worker
def make_train_loader(data_dir: str, indices: List[int], batch_size: int) -> DataLoader:

	train_dataset = datasets.MNIST(root=data_dir, train=True, download=False, transform=get_mnist_transform())

	#sottoinsieme del dataset con gli indici del worker
	local_dataset = Subset(train_dataset, indices)

	#DataLoader, shuffle true perchè mescola i dati locali
	train_loader = DataLoader(local_dataset, batch_size=batch_size, shuffle=True)

	return train_loader

#DataLoader di test
def make_test_loader(data_dir: str, batch_size: int) ->DataLoader:
	test_dataset = datasets.MNIST(root=data_dir, train=False, download=False, transform=get_mnist_transform())

	#shuffle false perchè nella fase di test non serve mescolare i dati
	test_loader = DataLoader(test_dataset, batch_size=batch_size, shuffle=False)

	return test_loader






















































