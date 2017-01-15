CC=gcc
LD=-lpthread -Os -s -Wl,-z,relro,-z,now
CP=cp -r
MAKE=make
CD=cd
DAEMON=ssdp
EXEC=ssdpd
RM=rm -f
SRC=src
INC=inc
BIN=bin
PKG=arch-armhf
DEB=fakeroot dpkg-deb --build
CHKDEB=lintian
LN=ln -s

.PHONY: clean deb lintian

$(BIN)/$(EXEC): $(SRC)/main.c $(SRC)/config.c $(SRC)/ssdp.c $(SRC)/network.c $(SRC)/http.c $(SRC)/textutil.c $(SRC)/logfilter.c
	$(CC) $(LD) -I $(INC) -o $@ $^ 

$(PKG)/usr/sbin/$(EXEC): $(BIN)/$(EXEC)
	$(CP) $< $@

clean:
	$(RM) $(BIN)/$(EXEC) && \
	$(RM) $(PKG)/usr/sbin/$(EXEC) && \
	$(RM) $(PKG).deb

deb: $(PKG).deb

$(PKG).deb: $(PKG)/usr/sbin/$(EXEC)
	$(DEB) $(PKG)

lintian: $(PKG).deb
	$(CHKDEB) $(PKG).deb || :

install-base: $(PKG)/usr/sbin/$(EXEC)
	$(CP) $< /usr/sbin/ && \
	$(CP) $(PKG)/etc/init.d/$(DAEMON) /etc/init.d/ && \
	$(CP) $(PKG)/etc/apache2/sites-available/site-ssdp /etc/apache2/sites-available && \
	$(CP) $(PKG)/usr/lib/ssdp /usr/lib/ssdp && \
	$(CD) /etc/apache2/sites-enabled && $(LN) ../sites-available/site-ssdp .

install-debian: install-base
	update-rc.d $(DAEMON) defaults

install-redhat: install-base
	chkconfig --add $(DAEMON) && \
	chkconfig $(DAEMON) on

