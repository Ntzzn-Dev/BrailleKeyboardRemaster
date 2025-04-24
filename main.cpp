/******************************************************************************
 * Projeto: BrailleKeyboard
 * Autor:   Nathan
 * GitHub:  https://github.com/Ntzzn-Dev
 * Data:    24/04/2025
 * Descrição:
 *   Transforma seu teclado convencional em um teclado em braille. O programa tem
 * seu próprio tradutor, usando o atalho Ctrl+Shift+U ele copia o texto ja traduzido para
 * sua área de transferencia, podendo colar ele em braille ou no alfabeto IndoArábico
 * usando o Ctrl+V.
 *  Possui os prefixos para letras maiusculas e numerais.
 *
 * Observações:
 *   - Funcional apenas no windows.
 *
 *****************************************************************************/

#include <windows.h>
#include <winuser.h>
#include <map>
#include <cctype>
#include <iostream>
#include <string>
#include <cwctype>
#include <thread>
#include <atomic>
#include <thread>
#include <algorithm>

std::map<wchar_t, wchar_t> brailleMap = {
    {L'a', L'\u2801'}, {L'b', L'\u2803'}, {L'c', L'\u2809'},
    {L'd', L'\u2819'}, {L'e', L'\u2811'}, {L'f', L'\u280B'},
    {L'g', L'\u281B'}, {L'h', L'\u2813'}, {L'i', L'\u280A'},
    {L'j', L'\u281A'}, {L'k', L'\u2805'}, {L'l', L'\u2807'},
    {L'm', L'\u280D'}, {L'n', L'\u281D'}, {L'o', L'\u2815'},
    {L'p', L'\u280F'}, {L'q', L'\u281F'}, {L'r', L'\u2817'},
    {L's', L'\u280E'}, {L't', L'\u281E'}, {L'u', L'\u2825'},
    {L'v', L'\u2827'}, {L'w', L'\u283A'}, {L'x', L'\u282D'},
    {L'y', L'\u283D'}, {L'z', L'\u2835'}, {L'ç', L'\u2809'},
    {L'1', L'\u2801'}, {L'2', L'\u2803'}, {L'3', L'\u2809'},
    {L'4', L'\u2819'}, {L'5', L'\u2811'}, {L'6', L'\u280B'},
    {L'7', L'\u281B'}, {L'8', L'\u2813'}, {L'9', L'\u280A'},
    {L'0', L'\u281A'}
};

std::map<wchar_t, wchar_t> reverseBrailleMap = {
    {L'\u2801', L'a'}, {L'\u2803', L'b'}, {L'\u2809', L'c'},
    {L'\u2819', L'd'}, {L'\u2811', L'e'}, {L'\u280B', L'f'},
    {L'\u281B', L'g'}, {L'\u2813', L'h'}, {L'\u280A', L'i'},
    {L'\u281A', L'j'}, {L'\u2805', L'k'}, {L'\u2807', L'l'},
    {L'\u280D', L'm'}, {L'\u281D', L'n'}, {L'\u2815', L'o'},
    {L'\u280F', L'p'}, {L'\u281F', L'q'}, {L'\u2817', L'r'},
    {L'\u280E', L's'}, {L'\u281E', L't'}, {L'\u2825', L'u'},
    {L'\u2827', L'v'}, {L'\u283A', L'w'}, {L'\u282D', L'x'},
    {L'\u283D', L'y'}, {L'\u2835', L'z'}
};

std::map<wchar_t, wchar_t> numberBrailleMap = {
    {L'\u2801', L'1'}, {L'\u2803', L'2'}, {L'\u2809', L'3'},
    {L'\u2819', L'4'}, {L'\u2811', L'5'}, {L'\u280B', L'6'},
    {L'\u281B', L'7'}, {L'\u2813', L'8'}, {L'\u280A', L'9'},
    {L'\u281A', L'0'}
};

std::map<wchar_t, wchar_t> prefixBrailleMap = {
    {L'n', L'\u283C'}, {L'm', L'\u2820'}
};

HHOOK hook;
#define MY_CUSTOM_EXTRA_INFO 0x12345678
bool novaNumeracao = false, necessitaVerify = false;

bool encontrarPrefixoNumerico(std::wstring ultimaPalavra){
    bool encontrou = false;

    if(!ultimaPalavra.empty() && ultimaPalavra.back() == L' '){
        return encontrou;
    }
    for (int i = static_cast<int>(ultimaPalavra.size()) - 1; i >= 0; --i) {
        if (ultimaPalavra[i] == prefixBrailleMap[L'n']) {
            encontrou = true;
            break;
        }
    }

    return encontrou;
}

bool encontrarPrefixoAlfabetico(const std::wstring& ultimaPalavra) {
    int prefixoEsperado = 0;

    for (size_t i = 0; i < ultimaPalavra.size(); ++i) {
        wchar_t ch = ultimaPalavra[i];

        if (ch == prefixBrailleMap[L'm']) { 
            prefixoEsperado = 2;
        } else if(prefixoEsperado == 2 && ch != prefixBrailleMap[L'm']){
            prefixoEsperado = 1;
        } else {
            return false;
        }
    }
    return true;
}

void GetTextBack() {
    INPUT inputs[6] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_SHIFT;
    inputs[0].ki.dwFlags = 0;
    inputs[0].ki.dwExtraInfo = (ULONG_PTR)MY_CUSTOM_EXTRA_INFO;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_CONTROL;
    inputs[1].ki.dwFlags = 0;
    inputs[1].ki.dwExtraInfo = (ULONG_PTR)MY_CUSTOM_EXTRA_INFO;

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_LEFT;
    inputs[2].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
    inputs[2].ki.dwExtraInfo = (ULONG_PTR)MY_CUSTOM_EXTRA_INFO;

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_LEFT;
    inputs[3].ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
    inputs[3].ki.dwExtraInfo = (ULONG_PTR)MY_CUSTOM_EXTRA_INFO;

    inputs[4].type = INPUT_KEYBOARD;
    inputs[4].ki.wVk = VK_CONTROL;
    inputs[4].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[4].ki.dwExtraInfo = (ULONG_PTR)MY_CUSTOM_EXTRA_INFO;

    inputs[5].type = INPUT_KEYBOARD;
    inputs[5].ki.wVk = VK_SHIFT;
    inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[5].ki.dwExtraInfo = (ULONG_PTR)MY_CUSTOM_EXTRA_INFO;

    SendInput(6, inputs, sizeof(INPUT));
}


std::wstring GetClp(){
    std::wstring text;

    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (!OpenClipboard(nullptr)) continue;

        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData != nullptr) {
            wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pszText) {
                text = pszText;
                GlobalUnlock(hData);
                CloseClipboard();
                break;
            }
        }
        CloseClipboard();
    }
    return text;
}

std::wstring GetSelectedText() {
    INPUT inputs[5] = {};
    
    // Garante que o shift não sera emitido junto ao ctrl+C
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_SHIFT;
    inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[0].ki.dwExtraInfo = (ULONG_PTR)MY_CUSTOM_EXTRA_INFO;
    
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_CONTROL;
    inputs[1].ki.dwFlags = 0;
    inputs[1].ki.dwExtraInfo = (ULONG_PTR)MY_CUSTOM_EXTRA_INFO;

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'C';
    inputs[2].ki.dwFlags = 0;
    inputs[2].ki.dwExtraInfo = (ULONG_PTR)MY_CUSTOM_EXTRA_INFO;

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = 'C';
    inputs[3].ki.dwFlags = 0 | KEYEVENTF_KEYUP;
    inputs[3].ki.dwExtraInfo = (ULONG_PTR)MY_CUSTOM_EXTRA_INFO;

    inputs[4].type = INPUT_KEYBOARD;
    inputs[4].ki.wVk = VK_CONTROL;
    inputs[4].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[4].ki.dwExtraInfo = (ULONG_PTR)MY_CUSTOM_EXTRA_INFO;

    SendInput(5, inputs, sizeof(INPUT));

    return GetClp();
}

std::wstring GetTranslatedText(std::wstring text){
    std::wstring translatedText;
    bool prefixNum = false;
    int prefixCaps = 0;

    for (wchar_t ch : text) {
        // Tradução para IndoArabico
        if (numberBrailleMap.count(ch) && prefixNum) {
            translatedText += numberBrailleMap[ch];
        } else if (reverseBrailleMap.count(ch)) {
            translatedText += prefixCaps > 0 ? towupper(reverseBrailleMap[ch]) : reverseBrailleMap[ch]; //Recebe a letra traduzida em maiusculo ou minusculo

            prefixNum = false;
            if(prefixCaps == 1){
                prefixCaps = 0;
            }
        } 
        // Tradução para braille
        else if (brailleMap.count(towlower(ch))) {
            if(iswupper(ch)){
                translatedText += prefixBrailleMap[L'm'];
            }
            if(std::isdigit(ch) && prefixNum == false){
                translatedText += prefixBrailleMap[L'n'];
                prefixNum = true;
            }
            translatedText += brailleMap[ch];
        } 
        // Prefixos
        else if(ch == L'\u283C'){
            prefixNum = true;
        } 
        else if(ch == L'\u2820'){
            if(prefixCaps == 1){
                prefixCaps = 2;
            } else {
                prefixCaps = 1;
            }
        } 
        else {
            translatedText += L' ';
            prefixCaps = 0;
            prefixNum = false;
        }
    }

    return translatedText;
}

void CopyToClipboard(const std::wstring& text) {
    bool sucesso = false;

    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (OpenClipboard(NULL)) {
            EmptyClipboard();

            size_t len = (text.size() + 1) * sizeof(wchar_t);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);

            if (hMem) {
                wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
                if (pMem) {
                    wcsncpy(pMem, text.c_str(), text.size() + 1);
                    GlobalUnlock(hMem);

                    if (SetClipboardData(CF_UNICODETEXT, hMem)) {
                        sucesso = true;
                    } else {
                        GlobalFree(hMem);
                    }
                } else {
                    GlobalFree(hMem);
                }
            }
            CloseClipboard();
            if (sucesso) break;
        }
    }
}

void removerCaractere(std::wstring& str, wchar_t caractere) {
    size_t pos;
    while ((pos = str.find(caractere)) != std::wstring::npos) {
        str.erase(pos, 1);
    }
}

void verificacaoMaiuscula(){
    std::wstring text = GetClp();

    //Garante que não há nada selecionado ja
    if(GetSelectedText() != text){
        return;
    }
    
    GetTextBack();
    std::wstring ultimaplv = GetSelectedText();
    if(encontrarPrefixoAlfabetico(ultimaplv)){
        removerCaractere(ultimaplv, prefixBrailleMap[L'm']);

        ultimaplv.insert(0, 2, prefixBrailleMap[L'm']);

        CopyToClipboard(ultimaplv);
        
        //Soltar seleção
        keybd_event(VK_CONTROL, 0, 0, 0);
        keybd_event('V', 0, 0, 0);
        keybd_event('V', 0, KEYEVENTF_KEYUP, 0);
        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
    } else {
        //Soltar seleção
        keybd_event(VK_RIGHT, 0, 0, 0);
        keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
    }

    CopyToClipboard(text);

    necessitaVerify = false;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        if (p->vkCode == VK_ESCAPE) {
            PostQuitMessage(0);
            return 1;
        }

        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
            (GetAsyncKeyState(VK_SHIFT) & 0x8000) &&
            wParam == WM_KEYDOWN && p->vkCode == L'U') {
                std::wstring translated = GetTranslatedText(GetSelectedText());
                CopyToClipboard(translated);
            return 1;
        }

        if (wParam == WM_KEYDOWN && !(p->dwExtraInfo == (ULONG_PTR)MY_CUSTOM_EXTRA_INFO)) {
            BYTE keyboardState[256];
            GetKeyboardState(keyboardState);

            // Corrigir os estados do das letras maiusculas
            keyboardState[VK_SHIFT] = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 0x80 : 0;
            keyboardState[VK_CAPITAL] = (GetKeyState(VK_CAPITAL) & 0x0001) ? 0x01 : 0;

            wchar_t unicodeChar[5];
            int result = ToUnicode(p->vkCode, p->scanCode, keyboardState, unicodeChar, 4, 0);

            if (result > 0) {
                wchar_t key = towlower(unicodeChar[0]);
                bool estaEmMaiusculo = std::isupper(unicodeChar[0]);

                if (key == L' ' || key == L'\r' || key == L'\b' || wParam == VK_LEFT || wParam == VK_RIGHT || wParam == VK_UP || wParam == VK_DOWN) {
                    novaNumeracao = true;
                    if(necessitaVerify){
                        verificacaoMaiuscula();
                    }
                } else {
                    if (!(GetAsyncKeyState(VK_CONTROL) & 0x8000) && brailleMap.count(key)) {
                        wchar_t brailleChar = brailleMap[key];
                        
                        if(std::isdigit(key) && novaNumeracao){
                            GetTextBack();
                            std::wstring ultimaplv = GetSelectedText();
                            keybd_event(VK_RIGHT, 0, 0, 0);
                            keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);

                            if(!encontrarPrefixoNumerico(ultimaplv)){
                                INPUT ip[2] = {};
                                ip[0].type = INPUT_KEYBOARD;
                                ip[0].ki.wScan = prefixBrailleMap[L'n'];
                                ip[0].ki.dwFlags = KEYEVENTF_UNICODE;
        
                                ip[1] = ip[0];
                                ip[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        
                                SendInput(2, ip, sizeof(INPUT));
                            }
                            novaNumeracao = false;
                        }
                        else if(!novaNumeracao && !(std::isdigit(key))){
                            novaNumeracao = true;
                        }

                        if(estaEmMaiusculo){
                            INPUT ip[2] = {};
                            ip[0].type = INPUT_KEYBOARD;
                            ip[0].ki.wScan = prefixBrailleMap[L'm'];
                            ip[0].ki.dwFlags = KEYEVENTF_UNICODE;
    
                            ip[1] = ip[0];
                            ip[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
    
                            SendInput(2, ip, sizeof(INPUT));

                            necessitaVerify = true;
                        }

                        INPUT ip[2] = {};
                        ip[0].type = INPUT_KEYBOARD;
                        ip[0].ki.wScan = brailleChar;
                        ip[0].ki.dwFlags = KEYEVENTF_UNICODE;

                        ip[1] = ip[0];
                        ip[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

                        SendInput(2, ip, sizeof(INPUT));
                        return 1;
                    }
                }
            }
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {}

    UnhookWindowsHookEx(hook);
    return 0;
}