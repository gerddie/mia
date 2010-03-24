SET(testplug_plugins 
	    dummy1 
            dummy2
	    lala 
            lolo
)

PLUGIN_GROUP_NOINST("${testplug_plugins}" "${MIACORE}")

ADD_CUSTOM_TARGET(test_core_plugins DEPENDS ${testplug_plugins})