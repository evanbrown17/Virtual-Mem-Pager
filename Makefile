# Makefile for pager project

CXX = g++
CXXFLAGS = -Wall -Werror -std=c++11 -g

pager: pager.cc
	$(CXX) $(CXXFLAGS) -o pager pager.cc libvm_pager.a -lssl -lcrypto

% : %.cc
	$(CXX) $(CXXFLAGS) -o $@ $@.cc libvm_app.a

clean:
	find . -type f -executable -delete

