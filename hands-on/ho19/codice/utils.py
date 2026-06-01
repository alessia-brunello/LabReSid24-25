from typing import Dict, List, Tuple
import torch

#dizionario che associa nomi di parametri a tensori
StateDict = Dict[str, torch.Tensor]

WorkerUpdate = Tuple[StateDict, int, float]

#copia uno state_dict sulla cpu
def state_dict_to_cpu(state_dict: StateDict) -> StateDict:

	cpu_state_dict = {}

	for name, tensor in state_dict.items():

		#detach separa il tensore dal grado dei gradienti;
		#cpu sposta il tenrore sulla cpu, clone crea una copia identica
		cpu_state_dict[name] = tensor.detach().cpu().clone()

	return cpu_state_dict


#implementazione di FedAg
def weighted_average(updates: List[WorkerUpdate]) -> StateDict:

	#se non ci sono aggiornamenti non si può aggregare nulla
	if len(updates) == 0:
		raise ValueError("La lista degli aggiornamenti dei worker è vuota.\n")

	#numero totale di esempi usati da tutti i worker
	total_samples = sum(num_samples for _, num_samples, _ in updates)

	#state_dict del primo worker riferimento per le chiavi
	first_state_dict = updates[0][0]

	averaged_state_dict = {}

	for key in first_state_dict.keys():

		if not torch.is_floating_point(first_state_dict[key]):
			averaged_state_dict[key] = first_state_dict[key].clone()
			continue

		#tensore di zeri
		averaged_tensor = torch.zeros_like(first_state_dict[key], dtype=torch.float32)

		for local_state_dict,  num_samples, _ in updates:

			#peso del worker nella media, uno con più dati conta di più
			worker_weight = num_samples/total_samples


			#somma del parametro locale pesato
			averaged_tensor += local_state_dict[key].float() * worker_weight

		#salvataggio del parametro medio nel nuovo state_dict globale
		averaged_state_dict[key] = averaged_tensor


	#modello globale aggregato
	return averaged_state_dict



#calcola la loss media tra i worker
def mean_worker_loss(updates: List[WorkerUpdate]) -> float:

	if len(updates) == 0:
		return 0.0

	#somma dei loss locali
	total_loss = sum(local_loss for _, _, local_loss in updates)

	#diviso il numero di worker
	return total_loss / len(updates)















