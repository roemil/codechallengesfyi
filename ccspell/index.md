# Spellchecker using Bloom filter
[Repo](https://github.com/roemil/codechallengesfyi/tree/master/ccspell)

## Bloom filter
A Bloom filter is a space-efficient probabilistic data structure, conceived by Burton Howard Bloom in 1970, that is used to test whether an element is a member of a set. False positive matches are possible, but false negatives are not – in other words, a query returns either "possibly in set" or "definitely not in set". Elements can be added to the set, but not removed (though this can be addressed with the counting Bloom filter variant); the more items added, the larger the probability of false positives.
[Wikipedia](https://en.wikipedia.org/wiki/Bloom_filter)

![Bloom filter](Bloom_filter.svg)

## Usage
* Compile and run ccspell
* Use "-build" to generate the bloom filter (./ccspell -build)
* To spellcheck words do: ./ccspell [WORD1 WORD2 WORD3]


