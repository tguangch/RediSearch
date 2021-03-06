all:
	$(MAKE) -C ./src all

test:
	$(MAKE) -C ./src $@

clean:
	$(MAKE) -C ./src $@

distclean:
	$(MAKE) -C ./src $@
.PHONY: distclean

package: all
	$(MAKE) -C ./src package
.PHONY: package

buildall:
	$(MAKE) -C ./src $@

deploydocs:
	mkdocs build
	s3cmd sync site/ s3://redisearch.io
.PHONY: deploydocs

staticlib:
	$(MAKE) -C ./src $@

docker: distclean
	docker build . -t redislabs/redisearch

docker_push: docker
	docker push redislabs/redisearch:latest