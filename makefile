.PHONY: docs
docs:
	rm -rf docs/html
	doxygen doxygen_config
