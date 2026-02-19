#---------------------------------------------------------------------------------
# App-Metadaten (Wichtig für das Homebrew-Menü)
#---------------------------------------------------------------------------------
APP_NAME        := GitHubWiiU
APP_AUTHOR      := Morris
APP_DESCRIPTION := Download files from github repositorys on your wiiu
APP_VERSION     := 1.0.0

#---------------------------------------------------------------------------------
# Build-Einstellungen
#---------------------------------------------------------------------------------
TARGET          := $(APP_NAME)
SOURCES         := src
RESOURCES       := res

# Nutze die WUT-Regeln (WiiU Toolchain)
include $(WUT_ROOT)/share/wut_rules.mk

# Bibliotheken für Internet & Wii U
# -lcurl für GitHub, -lssl/-lcrypto für HTTPS, -lwut für Systemfunktionen
LIBS            := -lcurl -lssl -lcrypto -lz -lwut -lutil

#---------------------------------------------------------------------------------
# WUHB-Konfiguration (Hier werden Name & Autor verpackt)
#---------------------------------------------------------------------------------
# Dies sorgt dafür, dass die .wuhb-Datei die Metadaten schluckt
WUHB_ARGS := --name "$(APP_NAME)" \
             --author "$(APP_AUTHOR)" \
             --description "$(APP_DESCRIPTION)" \
             --version "$(APP_VERSION)"

# Standard-Target: Erstellt die fertige .wuhb
all: $(TARGET).wuhb
