import threading

results = {}

def calc_A():
	results['A'] = 2 * 6

def calc_B():
	results['B'] = 1 + 4

def calc_C():
	results['C'] = 5 - 2

def calc_D():
	results['D'] = results['B'] * results['C']

def calc_Y():
	results['Y'] = results['A'] + results['D']

def main():
	#A,B,C
	threads = [
		threading.Thread(target=calc_A),
		threading.Thread(target=calc_B),
		threading.Thread(target=calc_C)
	]

	for t in threads:
		t.start()

	for t in threads:
		t.join()
	print("Grafo risolto in Python\n")
	print("A:", results['A'])
	print("\nB:", results['B'])
	print("\nC:", results['C'])


	#D
	tD = threading.Thread(target=calc_D)
	tD.start()
	tD.join()

	print("\nD = B * C -->", results['D'])

	#Y
	tY = threading.Thread(target=calc_Y)
	tY.start()
	tY.join()

	print("\nRisultato")
	print("\nY = A + D -->",results['Y'])

if __name__=="__main__":
	main()

