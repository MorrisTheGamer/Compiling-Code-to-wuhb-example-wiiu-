#include <wut.h>
#include <vpad/input.h>
#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <coreinit/screen.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

// Deine Datenstruktur vom 3DS
typedef struct {
    char name[128];
    char downloadUrl[256];
    bool isNewRelease;
} ListEntry;

ListEntry listItems[150];
int itemCount = 0, selectedIndex = 0;
char *api_buffer = NULL;
size_t api_buffer_size = 0;
bool isBusy = false;

// Hilfsfunktion für die GitHub API
size_t write_to_ram(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    char *new_ptr = (char*)realloc(api_buffer, api_buffer_size + total + 1);
    if (!new_ptr) return 0;
    api_buffer = new_ptr;
    memcpy(api_buffer + api_buffer_size, ptr, total);
    api_buffer_size += total;
    api_buffer[api_buffer_size] = '\0';
    return total;
}

void performApiRequest(const char* url) {
    isBusy = true;
    CURL *curl = curl_easy_init();
    if (api_buffer) { free(api_buffer); api_buffer = NULL; }
    api_buffer_size = 0;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "WiiU-GitHub-Client");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_ram);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    isBusy = false;
}

// Dein Parser (angepasst auf WiiU)
void parseRepos() {
    itemCount = 0; char *p = api_buffer; if (!p) return;
    while (p && (p = strstr(p, "\"full_name\":\"")) && itemCount < 100) {
        p += 13; char *end = strchr(p, '\"');
        if (end) {
            size_t len = end - p; if (len > 127) len = 127;
            strncpy(listItems[itemCount].name, p, len);
            listItems[itemCount].name[len] = '\0';
            itemCount++; p = end;
        }
    }
}

int main(int argc, char **argv) {
    WHBProcInit();
    WHBLogConsoleInit(); // Standard: Hell auf Dunkel (kein Darkmode nötig)

    VPADStatus vpad;
    VPADReadError error;
    bool showOnTV = false;

    // Start-Suche
    performApiRequest("https://api.github.com/search/repositories?q=WiiU+fork:true&per_page=50");
    parseRepos();

    while (WHBProcIsRunning()) {
        VPADRead(VPAD_CHAN_0, &vpad, 1, &error);

        if (error == VPAD_READ_SUCCESS) {
            // DEIN ZR-SWITCH: Wechselt den Fokus zum TV
            if (vpad.trigger & VPAD_BUTTON_ZR) {
                showOnTV = !showOnTV;
                WHBLogConsoleSetTarget(showOnTV ? WHB_LOG_CONSOLE_TV : WHB_LOG_CONSOLE_DRC);
            }

            // Navigation wie beim 3DS
            if (vpad.trigger & VPAD_BUTTON_DOWN && selectedIndex < itemCount - 1) selectedIndex++;
            if (vpad.trigger & VPAD_BUTTON_UP && selectedIndex > 0) selectedIndex--;
            
            if (vpad.hold & VPAD_BUTTON_HOME) break;
        }

        WHBLogConsoleClear();
        printf("GitHubWiiU Client - [ZR] Screen Wechsel\n");
        printf("=======================================\n\n");
        
        if (isBusy) {
            printf("Lade Repositories...\n");
        } else {
            for (int i = 0; i < itemCount && i < 15; i++) {
                if (i == selectedIndex) printf(" > %s\n", listItems[i].name);
                else printf("   %s\n", listItems[i].name);
            }
        }
        WHBLogConsoleDraw();
    }

    WHBLogConsoleFree();
    WHBProcShutdown();
    return 0;
}
