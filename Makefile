all: asio gl vk

asio:
	$(MAKE) -C asio

gl:
	$(MAKE) -C gl

vk:
	$(MAKE) -C vk

.PHONY: all asio gl vk
