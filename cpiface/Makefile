# SPDX-License-Identifier: Apache-2.0
# Copyright(c) 2019 Intel Corporation

CXX ?= g++
CXXFLAGS ?= -g -O3 -std=c++17 -Werror -Wall -Wcast-align
TARGET ?= zmq-cpiface
PBDIR ?= ./pb
GRPC_LIBS = -l:libprotobuf.a -l:libgrpc++.a -l:libgrpc_unsecure.a -l:libcares.a -lpthread -lz
SRCS := zmq-cpiface.cc

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -I$(PBDIR) $(PBDIR)/*.pb.o -o $(TARGET) $(GRPC_LIBS) -lzmq -lglog

clean:
	rm -rf $(TARGET)
