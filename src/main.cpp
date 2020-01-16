#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include <windows.h>
#include <ctime> //per introdurre un timer (clock()) e arrivare a bloccare i frame a 60FPS
#include <cstdlib>

#include <vector>
#include <list>

#include "Oggetto.h"


void framebuffer_size_callback(GLFWwindow* finestra, int width, int height) {
	glViewport(0, 0, width, height);
}
bool inizializzazione(GLFWwindow *&finestra) {
    //inizializzazione glfw per la creazione di finestre e la gestione degli input e degli eventi
	const unsigned int LARGHEZZA_SCHERMO = 900;
	const unsigned int ALTEZZA_SCHERMO = 500;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//inizializzazione finestra
	finestra = glfwCreateWindow(LARGHEZZA_SCHERMO, ALTEZZA_SCHERMO, "Asteroids Replica Lorenzo Cappellotto", NULL, NULL);
	if (finestra == NULL)	{
		std::cout<<"Errore durante la creazione della finestra GLFW"<<std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(finestra);
	glfwSetFramebufferSizeCallback(finestra, framebuffer_size_callback);

	if (glewInit())
		return false;

	return true;
}
bool caricaTestoShaders(std::string &testoShader, const char* nomeFileShader) {
    //funzione che trasferisce il codice della shader da un file a una stringa

	std::ifstream ShaderFile;
	std::stringstream ShaderStream;

	ShaderFile.open(nomeFileShader);
	if (!ShaderFile.is_open()) return false;

	ShaderStream << ShaderFile.rdbuf();
	ShaderFile.close();

	testoShader = ShaderStream.str();
	return true;
}
bool getShadersProgram(int &programmaShader, const char* testoVertexShader, const char* testoFragmentShader) {
	int vertexShader, fragmentShader, flagDiSuccesso;
	char logDiInfo[512];

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertexShader, 1, &testoVertexShader, NULL);
	glShaderSource(fragmentShader, 1, &testoFragmentShader, NULL);

	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &flagDiSuccesso);
	if (!flagDiSuccesso) {
		glGetShaderInfoLog(vertexShader, 512, NULL, logDiInfo);
		std::cout << "compilazione fallita vertex shader\n" << logDiInfo << std::endl;

		return false;
	}

	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &flagDiSuccesso);
	if (!flagDiSuccesso) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, logDiInfo);
		std::cout << "compilazione fallita fragment shader\n" << logDiInfo << std::endl;

		return false;
	}

	programmaShader = glCreateProgram();
	glAttachShader(programmaShader, vertexShader);
	glAttachShader(programmaShader, fragmentShader);
	glLinkProgram(programmaShader);

	glGetProgramiv(programmaShader, GL_LINK_STATUS, &flagDiSuccesso);
	if (!flagDiSuccesso) {
		glGetProgramInfoLog(programmaShader, 512, NULL, logDiInfo);
		std::cout << "liking fallito di programma shader\n" << logDiInfo << std::endl;

		return false;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return true;
}

void processaInput(GLFWwindow *finestra);
//so che non si fa cosi, da sistemare, variabili globali usate sia dal main che da
float gradi = 0.0f, velocita = 0.0f;
float velocitaMassimaNavicella = 0.03f;
float rotazioneMassimaNavicella = 5.0f;
int creaProiettile=0;

int main() {
    //una serie di variabili globali necessarie nel main

    //finestra attiva di gioco
    GLFWwindow* miaFinestra = NULL;

    //testo shader e programma
    std::string testoVertexShader, testoFragmentShader;
    int programmaShader;

    //variabili di gioco
    int punteggio = 0;
    int viteNavicella = 30;
    int numeroAsteroidiIniziali = 10;

    float velocitaProiettile = 0.05f;
    float velocitaAsteroide = 0.02f;
    /*************/
    //limite numero FPS e cronometri vari per far funzionare il limitatore di FPS
    int FPS = 60;
    clock_t inizioCronometro, fineCronometro;
    double secondiTrascorsi;
    /*************/

    //vertici, indici e posizioni oggetti in scena
    float verticiNavicella[] = {//vertici navicella
         0.9f,  1.0f, 0.0f,  0.0f,  0.0f,// alto-destra
         0.9f, -1.0f, 0.0f,  0.0f, -1.0f,// basso-destra
        -0.9f, -1.0f, 0.0f, -1.0f, -1.0f,// basso-sinistra
        -0.9f,  1.0f, 0.0f, -1.0f,  0.0f// alto-sinistra
    };
    unsigned int indiciNavicella[] = {0,1,2, 0,2,3};
    glm::vec3 posizioneNavicella = glm::vec3( 0.0f,  0.0f,  0.0f);

    float verticiAsteroide[] = {//vertici asteroide e proiettile
         0.5f,  0.5f, 0.0f,  1.0f,  1.0f,// alto-destra
         0.5f, -0.5f, 0.0f,  1.0f,  0.0f,// basso-destra
        -0.5f, -0.5f, 0.0f,  0.0f,  0.0f,// basso-sinistra
        -0.5f,  0.5f, 0.0f,  0.0f,  1.0f// alto-sinistra
    };
    unsigned int indiciAsteroide[] = {0,1,2, 0,2,3};

    float verticiProiettile[] = {//vertici asteroide e proiettile
         0.5f,  0.5f, 0.0f,   0.0f,  0.0f,// alto-destra
         0.5f, -0.5f, 0.0f,   0.0f, -1.0f,//basso-destra
        -0.5f, -0.5f, 0.0f,  -1.0f, -1.0f,// basso-sinistra
        -0.5f,  0.5f, 0.0f,  -1.0f,  0.0f// alto-sinistra
    };
    unsigned int indiciProiettile[] = {0,1,2, 0,2,3};

    glm::vec3 possibiliPosizioniAsteroidi[] = {
        glm::vec3(  0.0f,  -0.3f,  0.0f),
        glm::vec3( -0.1f,  -0.2f,  0.0f),
        glm::vec3(  0.2f,   0.1f,  0.0f),
        glm::vec3( -0.3f,   0.5f,  0.0f),
        glm::vec3(  0.4f,  -0.0f,  0.0f),
        glm::vec3( -0.5f,  -0.7f,  0.0f),
        glm::vec3(  0.6f,   0.0f,  0.0f),
        glm::vec3( -0.7f,   0.6f,  0.0f),
        glm::vec3(  0.8f,  -0.0f,  0.0f),
        glm::vec3( -0.9f,  -0.3f,  0.0f)
    };

	if (!inizializzazione(miaFinestra)) return -1;

	if (!caricaTestoShaders(testoVertexShader,"shaders/vertexShaderTransform.vert")) return -1;
	if (!caricaTestoShaders(testoFragmentShader,"shaders/fragmentShaderTexture.frag")) return -1;

	if (!getShadersProgram(programmaShader, testoVertexShader.c_str(), testoFragmentShader.c_str())) return -1;

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //va fatto dopo aver fatto shader program
    //questo server per ottenere il numero di vertici: sizeof(verticiNavicella)/sizeof(verticiNavicella[0])
    //creazione navicella e posizionamento
    Oggetto* navicella = new Oggetto(programmaShader,verticiNavicella,sizeof(verticiNavicella)/sizeof(verticiNavicella[0]),indiciNavicella,sizeof(indiciNavicella)/sizeof(indiciNavicella[0]),0.05f,"textures/navicella.jpg");
    navicella->setPosizione(posizioneNavicella);

    //creazione asteroidi e assegnazione coordinate, velocita e angolazione
    std::vector<Oggetto*> mioVettoreAsteroidi;
    for(int x=0; x<numeroAsteroidiIniziali; x++){//crea cinque asteroidi //da modificare
        mioVettoreAsteroidi.push_back(new Oggetto(programmaShader,verticiAsteroide,sizeof(verticiAsteroide)/sizeof(verticiAsteroide[0]),indiciAsteroide,sizeof(indiciAsteroide)/sizeof(indiciAsteroide[0]),0.05f,"textures/asteroide.jpg"));
    }

    srand((unsigned)time(NULL));
    for(unsigned int x=0; x<mioVettoreAsteroidi.size(); x++){//da sistemare il for
        float angoloTemp=(float)(rand()%360);
        int posizioneAsteroide=rand()%10;

        mioVettoreAsteroidi[x]->setAngolo(angoloTemp);
        mioVettoreAsteroidi[x]->setPosizione(possibiliPosizioniAsteroidi[posizioneAsteroide]);
        mioVettoreAsteroidi[x]->setVelocita(velocitaAsteroide);
    }

    std::vector<Oggetto*> mioVettoreProiettili;
    creaProiettile=-1;//-1 perche funziona in negativo il contatore, quando è a 0 aggiunge un proiettile

	while (!glfwWindowShouldClose(miaFinestra) && viteNavicella>0 && mioVettoreAsteroidi.size()>0) {
		/*************/
		inizioCronometro = clock();//mi segno il tempo all'inizio del frame
		/*************/

		processaInput(miaFinestra);
        if(creaProiettile==0){
            creaProiettile=-7;//prima era -3 ma per rallentare la possibilita di sparare ho alzato a 7 il numero di frame necessari in cui la barra deve essere premuta prima che lo sparo avvenga
            //se lo sparo avviene creo un oggetto proiettile con posizione e angolo della navicella e lo aggiungo alla lista di proiettili
            Oggetto* tempp=new Oggetto(programmaShader,verticiProiettile,sizeof(verticiProiettile)/sizeof(verticiProiettile[0]),indiciProiettile,sizeof(indiciProiettile)/sizeof(indiciProiettile[0]),0.01f,"textures/proiettile.jpg");
            tempp->setAngolo(navicella->getAngolo());
            tempp->setPosizione(navicella->getPosizione());
            tempp->setVelocita(velocitaProiettile);

            mioVettoreProiettili.push_back(tempp);
        }

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);//non da eseguire dentro la classe ma solo nel main una volta per frame
        glClear(GL_COLOR_BUFFER_BIT);

        //ogni frame aggiorno posizione della navicella, dei proiettili e degli asteroidi
        navicella->setVelocita(velocita);
        navicella->setAngolo(gradi);
        navicella->update();

        for(unsigned int x=0; x<mioVettoreProiettili.size(); x++){//da sistemare il for
            mioVettoreProiettili[x]->update();
        }

        for(unsigned int x=0; x<mioVettoreAsteroidi.size(); x++){//da sistemare il for
            mioVettoreAsteroidi[x]->update();
        }

        //a questo punto eseguo il rendering tenendo conto delle possibili collisioni e quindi dei possibili scenari
        navicella->render();

        for(unsigned int x=0; x<mioVettoreAsteroidi.size(); x++){//da sistemare il for
            bool render=true;
            //se un asteroide collide o con la navicella o con un proiettile deve essere eliminato e non renderizzato
            //aggiornando le variabili di gioco

            //for (std::vector<int>::iterator it = mioVettoreProiettili.begin() ; it != mioVettoreProiettili.end(); ++it){
            for(unsigned int i=0;i<mioVettoreProiettili.size();i++){
                if(mioVettoreAsteroidi[x]->collidesWith(mioVettoreProiettili[i])){
                    mioVettoreProiettili.erase(mioVettoreProiettili.begin()+i);
                    punteggio+=100;
                    render=false;
                    std::cout<<mioVettoreAsteroidi[x]->collidesWith(mioVettoreProiettili[i])<<"\n";
                    std::cout<<mioVettoreAsteroidi[x]->getPosizione().x<<" "<<mioVettoreAsteroidi[x]->getPosizione().y<<"\n";
                    std::cout<<mioVettoreProiettili[i]->getPosizione().x<<" "<<mioVettoreProiettili[i]->getPosizione().y<<"\n";
                    std::cout<<"numero asteroidi: "<<mioVettoreAsteroidi.size()<<" "<<"punteggio: "<<punteggio<<" "<<"vite: "<<viteNavicella<<"\n";
                }
            }
            if(mioVettoreAsteroidi[x]->collidesWith(navicella)){
                viteNavicella--;
                render=false;
                std::cout<<mioVettoreAsteroidi[x]->collidesWith(navicella)<<"\n";
                std::cout<<mioVettoreAsteroidi[x]->getPosizione().x<<" "<<mioVettoreAsteroidi[x]->getPosizione().y<<"\n";
                std::cout<<navicella->getPosizione().x<<" "<<navicella->getPosizione().y<<"\n";
                std::cout<<"numero asteroidi: "<<mioVettoreAsteroidi.size()<<" "<<"punteggio: "<<punteggio<<" "<<"vite: "<<viteNavicella<<"\n";
            }

            if(render) mioVettoreAsteroidi[x]->render();
            else mioVettoreAsteroidi.erase(mioVettoreAsteroidi.begin()+x);
        }
        //dato che i proiettili hanno durata limitata nel tempo controllo prima di renderizzare che non siano "vivi da troppo tempo"
        for(unsigned int x=0; x<mioVettoreProiettili.size(); x++){//da sistemare il for
            if(mioVettoreProiettili[x]->getTTL()>60) mioVettoreProiettili.erase(mioVettoreProiettili.begin()+x);
            else mioVettoreProiettili[x]->render();
        }

		glfwSwapBuffers(miaFinestra);
		glfwPollEvents();

        /*************/
        fineCronometro = clock();//mi segno il tempo alla fine del frame
		secondiTrascorsi = double(fineCronometro - inizioCronometro);//faccio la differenza dei due tempi per misurare il tempo impegato dal programma per ogni frame

		if(1000/FPS>=secondiTrascorsi)//attenzione senza if potrebbe bloccarsi perche' funzione sleep puo' ricevere un tempo negativo
            Sleep(1000/FPS-secondiTrascorsi);//aspetta per un frame da 1/60 di secondo in millisecondi - il tempo impiegato per il render
        /*************/
	}
    std::cout<<"numero asteroidi: "<<mioVettoreAsteroidi.size()<<" "<<"punteggio: "<<punteggio<<" "<<"vite: "<<viteNavicella<<"\n";

    //in caso di errore grafico ne stampa il numero, per debug
	GLenum errore;
    while((errore = glGetError()) != GL_NO_ERROR){
      std::cout<<errore;
    }
	return 0;
}
void processaInput(GLFWwindow *finestra) {
    //funzione usata per capire se alcuni bottoni sono stati premuti ed eseguire determinate azione alla pressione
    //se premo esc chiudi la finestra
	if (glfwGetKey(finestra, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(finestra, true);

    //se premo a-d o <-> fai ruotare la navicella intorno al suo asse
    if (glfwGetKey(finestra, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(finestra, GLFW_KEY_A) == GLFW_PRESS)
        gradi+=rotazioneMassimaNavicella;
    else if (glfwGetKey(finestra, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(finestra, GLFW_KEY_D) == GLFW_PRESS)
        gradi-=rotazioneMassimaNavicella;
    else
        gradi+=0.0f;
    //se premo w-s o ^-v fai avanzare la navicella
    if (glfwGetKey(finestra, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(finestra, GLFW_KEY_W) == GLFW_PRESS)
        velocita=+velocitaMassimaNavicella;
    else if (glfwGetKey(finestra, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(finestra, GLFW_KEY_S) == GLFW_PRESS)
        velocita=-velocitaMassimaNavicella;
    else
        velocita*=0.95f;
        //if(velocita<0.01f)
        // velocita=0.0f;

    //se premo spazio spara proiettili
    if (glfwGetKey(finestra, GLFW_KEY_SPACE) == GLFW_PRESS){
        creaProiettile++;
    }
}
