import torch
import torch.nn as nn
import torch.nn.functional as F

#definisce una rete Multi-Layer Perceptron (fully-connected)
class SimpleMLP(nn.Module):

	def __init__(self) -> None:

		#costruttore della classe padre nn.Module
		super().__init__()

		#primo livello (256 valori in uscita)
		self.fc1 = nn.Linear(28*28, 256)

		#secondo livello (riceve 256 valori e ne produce 128)
		self.fc2 = nn.Linear(256, 128)

		#terzo livello (riceve 128 valori e ne produce 10 che corrispondono alle 10 cifre)
		self.fc3 = nn.Linear(128, 10)

	#metodo forward descrive come i dati attraversano la rete
	def forward(self, x: torch.Tensor) -> torch.Tensor:
		#ogni immagine viene trasformata in vettore
		x = x.view(-1, 28*28)

		#si applica il primo livello e ReLU introduce non linearità nella rete
		x = F.relu(self.fc1(x))

		#secondo livello
		x = F.relu(self.fc2(x))

		#terzo livello
		x = self.fc3(x)

		#restituisce i logits (punteggi grezzi delle 10 cifre)
		return x





















