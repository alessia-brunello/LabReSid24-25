import ray
import torch
import torch.nn as nn
from model import SimpleMLP
from data import make_test_loader

from utils import StateDict
from utils import WorkerUpdate
from utils import state_dict_to_cpu
from utils import weighted_average

@ray.remote(num_cpus=0.5)
class Aggregator:

	#costruttore
	def __init__(self, data_dir: str, test_batch_size: int, device: str) -> None:

		self.device = torch.device(device)
		self.model = SimpleMLP().to(self.device)
		self.criterion = nn.CrossEntropyLoss()

		self.test_loader = make_test_loader(data_dir=data_dir, batch_size=test_batch_size)

	#restituisce i pesi globali correnti
	def get_global_weights(self) -> StateDict:

		return state_dict_to_cpu(self.model.state_dict())

	#aggrega gli aggiornamenti dei worker
	def aggregate(self, updates: list[WorkerUpdate]) -> None:

		new_global_state_dict = weighted_average(updates)

		self.model.load_state_dict(new_global_state_dict)


	#valuta il modello globale sul test set
	def evaluate(self):

		self.model.eval()

		#contatore delle predizioni corrette
		correct = 0

		#contatore degli esempi totali
		total = 0

		#accumulatore della loss di test
		total_loss = 0.0


		#calcolo dei gradienti disabilitato durante il test
		with torch.no_grad():

			for data, target in self.test_loader:

				data = data.to(self.device)
				target = target.to(self.device)

				#output del modello globale
				output = self.model(data)

				loss = self.criterion(output, target)

				total_loss += loss.item()

				_, predicted = torch.max(output, dim=1)


				#quante predizioni sono corrette
				correct += predicted.eq(target).sum().item()

				#numero di esempi del batch
				total += target.size(0)

		accuracy = correct / total

		average_loss = total_loss / len(self.test_loader)

		return accuracy, average_loss
















































