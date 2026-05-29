from collections import Counter

import ray

from text_utils import tokenize, filter_tokens

#conta le parole presenti in un singolo chunk, restituisce un dizionario parola:occorrenze nel chunk
def count_words_in_chunk(chunk: str, stopwords: set[str], min_len: int) -> dict[str, int]:
    tokens = tokenize(chunk)
    filtered_tokens = filter_tokens(tokens, stopwords, min_len)

    counter = Counter(filtered_tokens)

    return dict(counter)

#FASE MAP
#ray.remote trasforma questa funzione in una task Ray, cioè può eseguire più chiamate a questa funzione in parallelo
#ritorna un dizionario con i conteggi parziali del chunk
@ray.remote
def map_task(chunk: str, stopwords: tuple[str, ...], min_len: int) -> dict[str, int]:

    return count_words_in_chunk(chunk=chunk, stopwords=set(stopwords), min_len=min_len)


#FASE REDUCE
#prende la lista dei dizionari parziali e li fonde in un unico dizionario
@ray.remote
def reduce_task(partial_results: list[dict[str, int]]) -> dict[str, int]:

    total = Counter()

    for partial_count in partial_results:
        total.update(partial_count)

    return dict(total)
    
    
#Divide una lista in sottoliste per aggregare i risultati a gruppi invece che tutti insieme nella fase di Reduce
def split_list(items: list, batch_size: int) -> list[list]:

    batches = []

    for index in range(0, len(items), batch_size):
        batch = items[index:index + batch_size]
        batches.append(batch)

    return batches



def run_map_reduce(chunks: list[str], stopwords: set[str], min_len: int, reduce_batch_size: int) -> dict[str, int]:

    if not chunks:
        return{}

    if reduce_batch_size <= 0:
        raise ValueError("reduce_batch_size deve essere maggiore di zero")

    #le stopwords vengono trasformate in tupla per essere facilmente passate ai task Ray
    stopwords_tuple = tuple(sorted(stopwords))


	#FASE DI MAP
	#per ogni chunk viene lanciato un task Ray
    map_refs = []

    for chunk in chunks:
        ref = map_task.remote(chunk, stopwords_tuple, min_len) #restituisce un riferimento ad un risultato futuro
        map_refs.append(ref)


	#FASE REDUCE
    current_refs = map_refs #contiene riferimenti Ray ai risultati parziali
    
    #in questa fase si aggregano i parziali fino ad ottenere un unico risultato
    while len(current_refs) > 1:
        next_refs = []

        batches = split_list(current_refs, reduce_batch_size)

        for batch in batches:

            #ray.get(batch) aspetta che i risultati del batch siano pronti e restituisce i dizionari prodotti dai task precedenti
            partial_results = ray.get(batch) 
            reduce_ref = reduce_task.remote(partial_results)

            next_refs.append(reduce_ref)

        current_refs = next_refs

    #rimane un solo risultato
    final_result = ray.get(current_refs[0])

    return final_result
























