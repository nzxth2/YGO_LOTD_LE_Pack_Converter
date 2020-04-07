#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_GDI_IMPLEMENTATION
#include "nk_gui.h"

#include "nuklear/nuklear.h"
#include "nuklear/nuklear_gdi.h"
#include "window.h"
#include "filehandling.h"
#include "cards.h"
#include <algorithm>

GdiFont* font;
struct nk_context *ctx;
unsigned int windowWidth;
unsigned int windowHeight;


std::string infoString;
std::unordered_map<Short,std::string> ID_to_name;
std::unordered_map<std::string,Short> name_to_ID;


void SetupGui(WINDOW_DATA &windowData,unsigned int initialWindowWidth, unsigned int initialWindowHeight){
    font = nk_gdifont_create("Segoe UI", 18);
    windowWidth=initialWindowWidth;
    windowHeight=initialWindowHeight;
    ctx = nk_gdi_init(font, windowData.dc, initialWindowWidth, initialWindowHeight);
    
    LoadCards(ID_to_name,name_to_ID);
    
    infoString="";
    
}

void HandleInput(WINDOW_DATA &windowData){
        MSG msg;
        nk_input_begin(ctx);
        if (windowData.needs_refresh == 0) {
            if (GetMessageW(&msg, NULL, 0, 0) <= 0)
                windowData.running = 0;
            else {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            windowData.needs_refresh = 1;
        } else windowData.needs_refresh = 0;

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                windowData.running = 0;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            windowData.needs_refresh = 1;
        }
        nk_input_end(ctx);
        
}

int HandleEvent(const EVENT_DATA &eventData){
    switch (eventData.msg)
    {
	case WM_SIZE:
        windowWidth=LOWORD(eventData.lparam);
        windowHeight=HIWORD(eventData.lparam);
		return nk_gdi_handle_event(eventData.wnd, eventData.msg, eventData.wparam, eventData.lparam);
    default:
        return nk_gdi_handle_event(eventData.wnd, eventData.msg, eventData.wparam, eventData.lparam);
    }
    
}

void HandleGui(){
    if (nk_begin(ctx, "Demo", nk_rect(0, 0, windowWidth, windowHeight),
        0))
    {
        nk_layout_row_dynamic(ctx,90,1);
        if(nk_group_begin_titled(ctx, "group_convert", "Convert", NK_WINDOW_TITLE|NK_WINDOW_BORDER)){
            nk_layout_row_static(ctx, 0, 100, 2);
            if (nk_button_label(ctx, "BIN to TXT")){
                std::string packFilename=OpenFilename("YGO LOTD LE BIN Pack Files (*.bin)\0*.*\0");
                if (!packFilename.empty()){
                    std::ifstream packFile(packFilename,std::ios::in|std::ios::binary);
                    if (packFile.is_open()){
                        std::vector<short> commonIDs;
                        std::vector<short> rareIDs;
                        int commonCount=ReadShort(packFile);
                        int rareCount=ReadShort(packFile);
                        for (int i=0; i< commonCount;i++){
                            commonIDs.push_back(ReadShort(packFile));
                        }
                        for (int i=0; i< rareCount;i++){
                            rareIDs.push_back(ReadShort(packFile));
                        }
                        packFile.close();

                        std::string txtFilename=SaveFilename("YGO LOTD LE TXT Pack Files (*.txt)\0*.*\0");
                        if (!txtFilename.empty()){
                            std::ofstream txtFile(txtFilename);
                            if (txtFile.is_open()){
                                txtFile << "# common cards. first the card count, then the list\n";
                                txtFile << commonCount << "\n";
                                for (int i=0; i<commonCount; i++){
                                    txtFile << ID_to_name[commonIDs[i]] << "\n";
                                }
                                txtFile << "# rare cards. first the card count, then the list\n";
                                txtFile << rareCount << "\n";
                                for (int i=0; i<rareCount; i++){
                                    txtFile << ID_to_name[rareIDs[i]];
                                    if (i+1<rareCount){
                                        txtFile << "\n";
                                    }
                                }
                                txtFile.close();
                                infoString="Successfully exported the pack to "+txtFilename;
                            }else{
                                infoString="Unable to open " + txtFilename;
                            }
                        }else{
                            infoString="Canceled file dialog";
                        }
                    }else{
                        infoString="Unable to open " + packFilename;
                    }
                }else{
                    infoString="Canceled file dialog";
                }
            }
            if (nk_button_label(ctx, "TXT to BIN")){
                std::string txtFilename=OpenFilename("YGO LOTD LE TXT Pack Files (*.txt)\0*.*\0");
                if (!txtFilename.empty()){
                    std::ifstream txtFile(txtFilename);
                    if(txtFile.is_open()){
                        std::string currLine;
                        std::vector<std::string> lines;
                        while (std::getline(txtFile, currLine)){
                            std::string simplifiedString=SimplifyString(currLine);
                            if (!simplifiedString.empty() && simplifiedString.front()!='#'){ // skip comments and empty lines
                                lines.push_back(simplifiedString);
                            }
                        }
                        txtFile.close();
                        int commonCount=0;
                        std::vector<std::string> commonCards; 
                        int rareCount=0;
                        std::vector<std::string> rareCards;
                        bool readablePack=false;
                        std::vector<std::string> missingCommon;
                        std::vector<std::string> missingRare;
                        if (lines.empty()){
                                infoString="Pack contains no cards, please check your .txt file!";
                            }else{
                                if (std::all_of(lines[0].begin(), lines[0].end(), ::isdigit)){
                                    commonCount=std::stoi(lines[0]);
                                    if (lines.size()<1+commonCount+1){
                                        infoString="Could not read whole pack list. Make sure Common and Rare cards count is correct";
                                    }else{
                                        for (int i=1; i<1+commonCount; i++){
                                            std::string cardName=StringToLower(lines[i]);
                                            if (name_to_ID.count(cardName)==1){
                                                commonCards.push_back(cardName);
                                            }else{
                                                missingCommon.push_back(lines[i]);
                                            }
                                            
                                        }
                                        if (std::all_of(lines[1+commonCount].begin(), lines[1+commonCount].end(), ::isdigit)){
                                            rareCount=std::stoi(lines[1+commonCount]);
                                            if (lines.size()<1+commonCount+1+rareCount){
                                                infoString="Could not read whole pack list. Make sure Common and Rare cards count is correct";
                                            }else{
                                                for (int i=1+commonCount+1; i<1+commonCount+1+rareCount; i++){
                                                    std::string cardName=StringToLower(lines[i]);
                                                    if (name_to_ID.count(cardName)==1){
                                                        rareCards.push_back(cardName);
                                                    }else{
                                                        missingRare.push_back(lines[i]);
                                                    }
                                                    readablePack=true;
                                                }
                                            }
                                        }else{
                                            infoString="Could not read Rare cards count!";
                                        }
                                    }
                                }else{
                                    infoString="Could not read Common cards count!";
                                }
                            }
                        if (readablePack){
                            commonCount-=missingCommon.size();
                            rareCount-=missingRare.size();
                            std::string packFilename=SaveFilename("YGO LOTD LE BIN Pack Files (*.bin)\0*.*\0");
                            if (!packFilename.empty()){
                                std::ofstream packFile(packFilename,std::ios::out|std::ios::binary);
                                if (packFile.is_open()){
                                    WriteShort(packFile,commonCount);
                                    WriteShort(packFile,rareCount);
                                    for (int i=0;i< commonCount;i++){
                                        WriteShort(packFile,name_to_ID[commonCards[i]]);
                                    }
                                    for (int i=0;i< rareCount;i++){
                                        WriteShort(packFile,name_to_ID[rareCards[i]]);
                                    }
                                    packFile.close();
                                    if (missingCommon.empty() && missingRare.empty()){
                                        infoString="Successfully exported the pack to "+packFilename;
                                    }else{
                                        std::ofstream missingFile("missingCards.txt");
                                        if (missingFile.is_open()){
                                            for (int i=0;i<missingCommon.size();i++){
                                                missingFile << missingCommon[i] << "\n";
                                            }
                                            for (int i=0;i<missingRare.size();i++){
                                                missingFile << missingRare[i] << "\n";
                                            }
                                            missingFile.close();
                                            infoString="Partially exported pack to "+packFilename+", check missingCards.txt for omitted cards";
                                        }else{
                                            infoString="Partially exported pack to "+packFilename+", but some cards could not be identified";
                                        }
                                    }
                                }else{
                                    infoString="Unable to open " + packFilename;
                                }
                            }else{
                                infoString="Canceled file dialog";
                            }
                        }
                    }else{
                        infoString="Unable to open " + txtFilename;
                    }
                }else{
                    infoString="Canceled file dialog";
                }
                
            }
            nk_group_end(ctx);
        }
        
        nk_layout_row_dynamic(ctx,0,1);
        nk_edit_string_zero_terminated(ctx,NK_EDIT_INACTIVE,const_cast<char*>(("INFO: "+infoString).c_str()),INT_MAX,nk_filter_default);
    }
    nk_end(ctx);
}


void RenderGui(){
    nk_gdi_render(nk_rgb(30,30,30));
}

void CleanupGui(){
    nk_gdifont_del(font);
}

void UpdateWindowSize(unsigned int width, unsigned int height){
    windowWidth=width;
    windowHeight=height;
}