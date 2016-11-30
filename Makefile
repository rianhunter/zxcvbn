# TODO: call this from package.json

CXX=emcc
CXXFLAGS=-std=c++14 -Inative-src -Wall -Wextra -Werror
CXXLDFLAGS=-std=c++14 --bind --memory-init-file 0

ifdef RELEASE
CXXFLAGS += -Oz -flto -DNDEBUG
CXXLDFLAGS += -Oz -flto --closure 1
endif

ifdef RELEASE
BUILD_SUFFIX=.RELEASE.o
else
BUILD_SUFFIX=.o
endif

ADJACENCY_GRAPHS_SOURCES=adjacency_graphs_js_bindings.cpp adjacency_graphs.cpp
MATCHING_SOURCES=matching_js_bindings.cpp matching.cpp scoring.cpp adjacency_graphs.cpp frequency_lists.cpp js_frequency_lists.cpp util.cpp
SCORING_SOURCES=scoring_js_bindings.cpp scoring.cpp adjacency_graphs.cpp frequency_lists.cpp js_frequency_lists.cpp util.cpp
ZXCVBN_SOURCES=zxcvbn_js_bindings.cpp matching.cpp scoring.cpp adjacency_graphs.cpp frequency_lists.cpp js_frequency_lists.cpp util.cpp time_estimates.cpp feedback.cpp

PREFIX=native-src/zxcvbn

ADJACENCY_GRAPHS_OBJECTS=$(addprefix $(PREFIX)/, $(ADJACENCY_GRAPHS_SOURCES:.cpp=$(BUILD_SUFFIX)))
ADJACENCY_GRAPHS_EXE=lib/adjacency_graphs.js

MATCHING_OBJECTS=$(addprefix $(PREFIX)/, $(MATCHING_SOURCES:.cpp=$(BUILD_SUFFIX)))
MATCHING_EXE=lib/matching.js

SCORING_OBJECTS=$(addprefix $(PREFIX)/, $(SCORING_SOURCES:.cpp=$(BUILD_SUFFIX)))
SCORING_EXE=lib/scoring.js

ZXCVBN_OBJECTS=$(addprefix $(PREFIX)/, $(ZXCVBN_SOURCES:.cpp=$(BUILD_SUFFIX)))
ZXCVBN_EXE=lib/zxcvbn.js

.PHONY : clean
clean:
	-rm -f $(MATCHING_EXE)
	-rm -f $(addprefix $(PREFIX)/, $(MATCHING_SOURCES:.cpp=.o))
	-rm -f $(addprefix $(PREFIX)/, $(MATCHING_SOURCES:.cpp=.RELEASE.o))
	-rm -f $(SCORING_EXE)
	-rm -f $(addprefix $(PREFIX)/, $(SCORING_SOURCES:.cpp=.o))
	-rm -f $(addprefix $(PREFIX)/, $(SCORING_SOURCES:.cpp=.RELEASE.o))
	-rm -f $(ADJACENCY_GRAPHS_EXE)
	-rm -f $(addprefix $(PREFIX)/, $(ADJACENCY_GRAPHS_SOURCES:.cpp=.o))
	-rm -f $(addprefix $(PREFIX)/, $(ADJACENCY_GRAPHS_SOURCES:.cpp=.RELEASE.o))
	-rm -f lib/pre.js lib/_frequency_lists.inc.js
	-rm -f $(PREFIX)/_frequency_lists.hpp $(PREFIX)/_frequency_lists.cpp
	-rm -f $(PREFIX)/adjacency_graphs.hpp $(PREFIX)/adjacency_graphs.cpp
	-rm -f $(ZXCVBN_EXE)
	-rm -f $(addprefix $(PREFIX)/, $(ZXCVBN_SOURCES:.cpp=.o))
	-rm -f $(addprefix $(PREFIX)/, $(ZXCVBN_SOURCES:.cpp=.RELEASE.o))


.PHONY: test
test: test-matching test-scoring

.PHONY: test-matching
test-matching: $(MATCHING_EXE) $(ADJACENCY_GRAPHS_EXE)
	ROOT="../lib" node_modules/.bin/coffeetape test/test-matching.coffee | node_modules/.bin/faucet

.PHONY: test-scoring
test-scoring: $(MATCHING_EXE) $(ADJACENCY_GRAPHS_EXE) $(SCORING_EXE)
	ROOT="../lib" node_modules/.bin/coffeetape test/test-scoring.coffee | node_modules/.bin/faucet

lib:
	mkdir lib

$(MATCHING_EXE): $(MATCHING_OBJECTS) lib
	$(CXX) $(CXXLDFLAGS) --pre-js lib/pre.js $(MATCHING_OBJECTS) -o $@

$(SCORING_EXE): $(SCORING_OBJECTS) lib
	$(CXX) $(CXXLDFLAGS) --pre-js lib/pre.js $(SCORING_OBJECTS) -o $@

$(ADJACENCY_GRAPHS_EXE): $(ADJACENCY_GRAPHS_OBJECTS) lib
	$(CXX) $(CXXLDFLAGS) $(ADJACENCY_GRAPHS_OBJECTS) -o $@

$(ZXCVBN_EXE): $(ZXCVBN_OBJECTS) zxcvbn_post.js lib
	$(CXX) $(CXXLDFLAGS) --pre-js lib/pre.js -s 'MODULARIZE=1' -s 'EXPORT_NAME="zxcvbn"' $(ZXCVBN_OBJECTS) -o $@
	mv $@ $@.tmp
	cat $@.tmp zxcvbn_post.js > $@
	rm $@.tmp

$(MATCHING_EXE) $(SCORING_EXE) $(ZXCVBN_EXE): lib/pre.js

lib/pre.js: lib/_frequency_lists.inc.js lib
	echo "var Module = eval('(function() { try { return Module || {} } catch(e) { return {} } })()'); Module['_frequency_lists'] = " > $@
	cat lib/_frequency_lists.inc.js >> $@
	echo ";" >> $@

$(PREFIX)/_frequency_lists.hpp:
	python data-scripts/build_frequency_lists.py data/ $@

lib/_frequency_lists.inc.js: lib
	python data-scripts/build_frequency_lists.py data/ $@

$(PREFIX)/adjacency_graphs.hpp:
	python data-scripts/build_keyboard_adjacency_graphs.py $@

$(PREFIX)/adjacency_graphs.cpp:
	python data-scripts/build_keyboard_adjacency_graphs.py $@

$(ZXCVBN_OBJECTS) $(ADJACENCY_GRAPHS_OBJECTS) $(SCORING_OBJECTS) $(MATCHING_OBJECTS): $(PREFIX)/adjacency_graphs.hpp $(PREFIX)/_frequency_lists.hpp $(shell find $(PREFIX) -type f -name '*.hpp')

%$(BUILD_SUFFIX) : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
