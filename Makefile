.PHONY: clean deb lintian

CC        = gcc
CFLAGS    = -Os -s
CPPFLAGS  = -Iinc
LDFLAGS   = -Wl,-z,relro,-z,now
LDLIBS    = -lpthread
CP        = cp -r
CD        = cd
RM        = rm -f
LN        = ln -s
MD        = mkdir -p
SRC       = src
BIN       = bin

DAEMON    = ssdp
EXEC      = tiny-ssdpd
OBJS      = $(SRC)/main.o $(SRC)/config.o $(SRC)/ssdp.o $(SRC)/network.o \
	    $(SRC)/http.o $(SRC)/textutil.o $(SRC)/logfilter.o
PKG       = arch-armhf
DEB       = fakeroot dpkg-deb --build
CHKDEB    = lintian

.c.o:
	@printf "  CC      $@\n"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

all: $(BIN)/$(EXEC)

$(BIN)/$(EXEC): $(OBJS)
	@printf "  LINK    $@\n"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean:
	@printf "  CLEAN   $(EXEC)\n"
	-@$(RM) $(OBJS) $(BIN)/$(EXEC)   \
		$(PKG)/usr/sbin/$(EXEC)  \
		$(PKG).deb

deb: $(PKG).deb

$(PKG).deb: $(BIN)/$(EXEC)
	$(MD) $(PKG)/usr/sbin/ && \
	$(CP) $(BIN)/$(EXEC) $(PKG)/usr/sbin/$(EXEC) && \
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

