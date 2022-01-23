.PHONY: clean All

All:
	@echo "----------Building project:[ EDSEngine - Debug ]----------"
	@$(MAKE) -f  "EDSEngine.mk"
	@echo "----------Building project:[ EDSEngineTestApp - Debug ]----------"
	@cd "EDSEngineTestApp" && $(MAKE) -f  "EDSEngineTestApp.mk"
clean:
	@echo "----------Cleaning project:[ EDSEngine - Debug ]----------"
	@$(MAKE) -f  "EDSEngine.mk"  clean
	@echo "----------Cleaning project:[ EDSEngineTestApp - Debug ]----------"
	@cd "EDSEngineTestApp" && $(MAKE) -f  "EDSEngineTestApp.mk" clean
