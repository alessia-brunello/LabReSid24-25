from pathlib import Path
import re

#la punteggiatura viene ignorata e i numeri non sono considerati parole. Vengono contate parole con lettere accentate

WORD_REGEX = re.compile(r"[^\W\d_]+(?:['’][^\W\d_]+)?", re.UNICODE)

#legge il contenuto del file di testo, vengono provate più codifiche per eventuali caratteri speciali
def read_text_file(file_path: Path) -> str:
	if not file_path.exists():
		raise FileNotFoundError(f"File non trovato: {file_path} ")

	if not file_path.is_file():
		raise ValueError(f"Il percorso indicato non è un file: {file_path}")

	encodings = ["utf-8", "utf-8-sig", "latin-1"]

	for encoding in encodings:
		try:
			return file_path.read_text(encoding=encoding)
		except UnicodeDecodeError:
			continue

	raise UnicodeDecodeError(
		"unknown",
		b"",
		0,
		1,
		f"Impossibile leggere il file {file_path}",
	)

#divide il testo in blocchi (chunk), ognuno viene elaborato separatamente da un task Ray. Restituisce una lista di chunk
def split_into_chunks(text: str, chunk_size: int) -> list[str]:

	if chunk_size <= 0:
		raise ValueError("chunk_size deve essere maggiore di zero")

	chunks = []
	start = 0
	text_lenght = len(text)

	while start < text_lenght:
		end = min(start + chunk_size, text_lenght)

		if end < text_lenght:
			while end < text_lenght and not text[end].isspace():
				end += 1

		chunk = text[start:end].strip()

		if chunk:
			chunks.append(chunk)

		start = end

		while start < text_lenght and text[start].isspace():
			start += 1

	return chunks



#estrae le parole dal testo, ignora la punteggiatura, rende tutto minuscolo e restituisce una lista di parole
def tokenize(text: str) -> list[str]:
	words = []

	for match in WORD_REGEX.finditer(text):
		word = match.group(0).lower()
		words.append(word)

	return words

#carica il file opzionale delle stopwords, ritorna un insieme di parole da ingorare
def load_stopwords(file_path: Path | None) -> set[str]:

	if file_path is None:
		return set()

	content = read_text_file(file_path)

	stopwords = set()

	for line in content.splitlines():
		word = line.strip().lower()

		if word and not word.startswith("#"):
			stopwords.add(word)

	return stopwords



#elimina le parole più corte di min_len e le parole rpesenti nel file stopwords, restituisce la lista di parole filtrate
def filter_tokens(tokens: list[str], stopwords: set[str], min_len: int,) -> list[str]:
	filtered = []

	for token in tokens:
		if len(token) >= min_len and token not in stopwords:
			filtered.append(token)

	return filtered























