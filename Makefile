REFPKGS  = cmd
SRCPKGS  = cir sat util
LIBPKGS  = $(REFPKGS) $(SRCPKGS)
MAIN     = main
SERVER   = server
LIBS     = $(addprefix -l, $(LIBPKGS))
SRCLIBS  = $(addsuffix .a, $(addprefix lib, $(SRCPKGS)))

EXEC     = fraig
SERVER_EXEC = fraig_server

server_all: libs server

main_all: libs main

libs:
	@for pkg in $(SRCPKGS); \
	do \
		echo "Checking $$pkg..."; \
		cd src/$$pkg; make -f make.$$pkg --no-print-directory PKGNAME=$$pkg; \
		cd ../..; \
	done

main:
	@echo "Checking $(MAIN)..."
	@cd src/$(MAIN); \
		make -f make.$(MAIN) --no-print-directory INCLIB="$(LIBS)" EXEC=$(EXEC);
	@ln -fs bin/$(EXEC) .

server:
	@echo "Checking $(SERVER)..."
	@cd src/$(SERVER); \
		make -f make.$(SERVER) --no-print-directory INCLIB="$(LIBS)" EXEC=$(SERVER_EXEC);
	@ln -fs bin/$(SERVER_EXEC) .

clean:
	@for pkg in $(SRCPKGS); \
	do \
		echo "Cleaning $$pkg..."; \
		cd src/$$pkg; make -f make.$$pkg --no-print-directory PKGNAME=$$pkg clean; \
                cd ../..; \
	done
	@echo "Cleaning $(MAIN)..."
	@cd src/$(MAIN); make -f make.$(MAIN) --no-print-directory clean
	@echo "Cleaning $(SERVER)..."
	@cd src/$(SERVER); make -f make.$(SERVER) --no-print-directory clean
	@echo "Removing $(SRCLIBS)..."
	@cd lib; rm -f $(SRCLIBS)
	@echo "Removing $(EXEC), $(SERVER_EXEC)..."
	@rm -f bin/$(EXEC) bin/$(SERVER_EXEC)

cleanall: clean
	@echo "Removing bin/*..."
	@rm -f bin/*

ctags:	  
	@rm -f src/tags
	@for pkg in $(SRCPKGS); \
	do \
		echo "Tagging $$pkg..."; \
		cd src; ctags -a $$pkg/*.cpp $$pkg/*.h; cd ..; \
	done
	@echo "Tagging $(MAIN)..."
	@cd src; ctags -a $(MAIN)/*.cpp $(MAIN)/*.h
	@echo "Tagging $(SERVER)..."
	@cd src; ctags -a $(SERVER)/*.cpp $(SERVER)/*.h

linux mac:
	@for pkg in $(REFPKGS); \
	do \
	        cd lib; ln -sf lib$$pkg-$@.a lib$$pkg.a; cd ../..; \
	done
	@cd ref; ln -sf $(EXEC)-$@ $(EXEC);
