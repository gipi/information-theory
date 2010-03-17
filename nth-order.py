#!/usr/bin/env python

import sys, random

class NthOrder():
	def __init__(self, order, text_length, stream=sys.stdin):
		self.order = order
		self.stream = stream
		self.list_of_combinations = {}
		self.generated_text_length = text_length
		self.generated_text = ""

		self.abominio = ['\n', '\r', '-']

	def parse(self):
		while 1:
			# read until EOF
			text = self.stream.read()

			# in EOF case exit
			if text == "":
				break

			text = text.lower()
			for char in self.abominio:
				text = text.replace(char, '')

			self.text = text

		self._parse_all_combinations()
		#for key, value in self.list_of_combination.iteritems():
		#	print key, "\t", value
		self.generated_text = self._seed_from_combinations()
		sys.stdout.write('%s' % self.generated_text)
		while len(self.generated_text) < self.generated_text_length:
			new_char = self._give_me_a_char(self.generated_text[-(self.order - 1):])

			sys.stdout.write('%c' % new_char)
			sys.stdout.flush()
			self.generated_text += new_char

		#print self.generated_text

	def _parse_all_combinations(self):
		idx = 0
		while idx < (len(self.text) - self.order):
			comb = self.text[idx:idx + self.order]
			if not self.list_of_combinations.has_key(comb):
				self.list_of_combinations[comb] = 1
			else:
				self.list_of_combinations[comb] += 1

			idx += 1

	def _seed_from_combinations(self):
		"""
		Return a seed (without trailing whitespace).
		"""
		seed = " x"
		while seed[0] == " " and seed[-1] != " ":
			seed = random.choice(self.list_of_combinations.keys())

		return seed

	def _give_me_a_char(self, last):
		"""
		Return the new character to add.

			last are the last (order - 1) characters.
		"""
		partial_list = []
		for key in self.list_of_combinations.keys():
			if key[:self.order - 1] == last:
				partial_list.append(key)

		if partial_list:
			new_seed = random.choice(partial_list)
		else:
			new_seed = 'x'

		return new_seed[-1]


if __name__ == "__main__":
	if len(sys.argv) < 3:
		print 'usage', sys.argv[0], '<order> <text length>'
		sys.exit(1)

	parser = NthOrder(int(sys.argv[1]), int(sys.argv[2]))
	parser.parse()
