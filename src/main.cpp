#include <temgi/Console.h>

#include <SDLPlatform.h>

int main(int argc, char* argv[]){
    if(argc != 2){
        return 1;
    }

    temgi::Console console;

    SDLPlatform platform(console);

    console.subscribe(platform);

    if(!console.loadCartridge(argv[1])){
        return 1;
    }

    console.run();

    console.unloadCartridge();

    return 0;
}