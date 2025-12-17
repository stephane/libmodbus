INCLUDEDIRS := src
PRIV_INCLUDEDIRS := ../
SRCDIRS := src

COMPONENT_PRIV_INCLUDEDIRS = $(addprefix libmodbus/, \
	$(PRIV_INCLUDEDIRS) \
	)

COMPONENT_SRCDIRS = $(addprefix libmodbus/, \
	$(SRCDIRS) \
	)

COMPONENT_ADD_INCLUDEDIRS = $(addprefix libmodbus/, \
	$(INCLUDEDIRS) \
	)



