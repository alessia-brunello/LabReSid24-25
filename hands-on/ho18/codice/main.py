from pathlib import Path
import argparse
from collections import Counter

import ray

from text_utils import read_text_file, split_into_chunks, load_stopwords
from mapreduce import run_map_reduce

PRINT_TOP = 10 #Numero di parole da stampare di default

#legge gli argomenti passati da terminale e restituisce un oggetto con tutti gli argomenti letti
def parse_args() -> argparse.Namespace:

    parser = argparse.ArgumentParser(description="MapReduce con Ray.io per contare le parole in LOTR.")

    parser.add_argument("--input", type=Path, default=Path("lotr.txt"), help="File di input contenente il testo LOTR.")

    parser.add_argument("--output", type=Path, default=Path("word_count.txt"), help="File di output con le occorrenze delle parole.")

    parser.add_argument("--chunk-size", type=int, default=20_000, help="Dimensione approssimativa di ogni chunk di testo.")

    parser.add_argument("--top", type=int, default=100, help="Numero di parole più frequenti da salvare. Usare 0 per salvare tutte le parole.")
    
    parser.add_argument("--print-top", type=int, default=PRINT_TOP, help="Numero di parole più frequenti da mostrare a video")

    parser.add_argument("--min-len", type=int, default=1, help="Lunghezza minima delle parole da contare.")

    parser.add_argument("--stopwords", type=Path, default=None, help="File opzionale con le stopword da ignorare.")

    parser.add_argument("--reduce-batch-size", type=int, default=16, help="Numero di risultati parziali aggregati per ogni reduce")

    parser.add_argument("--num-cpus", type=int, default=None, help="Numero di CPU locali da usare con Ray")

    parser.add_argument("--ray-address", type=str, default=None, help="Indirizzo di un cluster Ray. Lasciare vuoto per esecuzione locale")

    return parser.parse_args()



#la funzione scrive sul file i conteggi delle parole
def write_word_count(counts: dict[str, int], output_file: Path, top_n: int) -> None:

    counter = Counter(counts)

    #top_n è il numero di parole da salvare, se vale 0 salva tutto
    if top_n == 0:
	    most_common_words = counter.most_common()

    else:
	    most_common_words = counter.most_common(top_n)

    with output_file.open("w", encoding="utf-8") as file:
        file.write("termine, occorrenza\n")

        for word, occurrences in most_common_words:
            file.write(f"{word}, {occurrences}\n")


def main() -> None:

    args = parse_args()

    print("----AVVIO----\n")
    print("Lettura del file di input ...\n")
    text = read_text_file(args.input)

    print("Divisione del testo in chunk ...\n")
    chunks = split_into_chunks(text, args.chunk_size)

    print(f"Chunk creati: {len(chunks)}\n")

    print("Caricamento stopword ...")
    stopwords = load_stopwords(args.stopwords)

    if stopwords:
	    print(f"Stopword caricate: {len(stopwords)}\n")
    else:
        print("Nessuna stopword caricata.\n")

    print("----Avvio di RAY----")

    ray.init(address=args.ray_address, num_cpus=args.num_cpus, ignore_reinit_error=True, include_dashboard=False, log_to_driver=False)

    try:
	    print("\n----Esecuzione MAPREDUCE----\n")

	    counts = run_map_reduce(chunks=chunks, stopwords=stopwords, min_len=args.min_len, reduce_batch_size=args.reduce_batch_size)

	    total_words = sum(counts.values())
	    unique_words = len(counts)

	    print("Scrittura del file di output ...\n")
	    write_word_count(counts=counts, output_file=args.output, top_n=args.top)

	    print("COMPLETATO.\n")
	    print(f"Parole totali contate: {total_words}\n")
	    print(f"Parole distinte: {unique_words}\n")
	    print(f"File creato: {args.output}\n")

	    print(f"Top {args.print_top} parole:")

	    for word, occurences in Counter(counts).most_common(args.print_top):
		    print(f" {word}: {occurences}")
		
    finally:		
	    ray.shutdown()

if __name__ == "__main__":
    main()





































