all: build run

poi: build run

build:
	@csc poi.scm

run:
	@./poi
