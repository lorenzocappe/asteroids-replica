#include "Oggetto.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>

//vari costruttori
Oggetto::Oggetto(int programmaShader, float *verticiOggetto,int numeroVertici,unsigned int *indiciOggetto, int numeroIndici, float scalaOggetto,const char *percorsoFileTexture){
    this->setProgrammaShader(programmaShader);
    this->setVerticiOggetto(verticiOggetto,numeroVertici,indiciOggetto,numeroIndici);
    this->setTexture(percorsoFileTexture);
    this->scala=scalaOggetto;

    matrixModelTrasformation=glm::mat4(1.0);//non sicuro del funzionamento
    matrixViewTrasformation=glm::mat4(1.0);
    matrixProjectionTrasformation=glm::mat4(1.0);
    setPosizione(glm::vec3(0.0f,0.0f,0.0f));
    setAngolo(0.0f);
    timeToLive=0;
}
Oggetto::Oggetto(int programmaShader, float *verticiOggetto,int numeroVertici,unsigned int *indiciOggetto, int numeroIndici, float scalaOggetto){
    Oggetto(programmaShader,verticiOggetto,numeroVertici,indiciOggetto,numeroIndici,scalaOggetto,"");
}
Oggetto::Oggetto(float *verticiOggetto,int numeroVertici, unsigned int *indiciOggetto, int numeroIndici){
    Oggetto(0,verticiOggetto,numeroVertici,indiciOggetto,numeroIndici,1.0f);
}
Oggetto::Oggetto(int programmaShader){
    Oggetto(programmaShader,NULL,0,NULL,0,1.0f);
}
Oggetto::Oggetto(){
    Oggetto(0);
}

//i set importanti
void Oggetto::setPosizione(glm::vec3 nuovo){
    this->posizione=nuovo;
}
void Oggetto::setProgrammaShader(int programmaShader){
    this->programmaShaderDaUtilizzare = programmaShader;
    if(issetProgrammaShader()){
        this->locationMatrixModelTrasformation = glGetUniformLocation(programmaShaderDaUtilizzare, "model");
        this->locationMatrixViewTrasformation = glGetUniformLocation(programmaShaderDaUtilizzare, "view");
        this->locationMatrixProjectionTrasformation = glGetUniformLocation(programmaShaderDaUtilizzare, "projection");
        //this->locationMatrixModelTrasformation = glGetUniformLocation(programmaShaderDaUtilizzare, "ourTexture");//apparentemente non funziona se inserito qui
    }
}
bool Oggetto::setTexture(const char *percorsoFileTexture){
    if(percorsoFileTexture==NULL) return false;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // carica immagine, crea la texture e genera i mipmaps

    int larghezza, altezza, nrChannels;
    stbi_set_flip_vertically_on_load(true); // dice a stb_image.h di ruotare la texture secondo l'asse delle y
    unsigned char *data;
    data = stbi_load(percorsoFileTexture, &larghezza, &altezza, &nrChannels, 0);

    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, larghezza, altezza, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else{
        std::cout << "Errore nel caricamento della texture" << std::endl;
        return false;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}
void Oggetto::setVerticiOggetto(float *verticiOggetto,int numeroVertici,unsigned int *indiciOggetto,int numeroIndici){
    this->verticiOggetto = verticiOggetto;
    this->indiciOggetto = indiciOggetto;
    this->numeroVertici = numeroVertici;

    if(issetVerticiOggetti()){
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        //array contiene vertici e indici relativi
        glBindVertexArray(VAO);
        //questo buffer contiene i semplici vertici
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(verticiOggetto)*numeroVertici, verticiOggetto, GL_STREAM_DRAW);
        //sizeof(verticiOggetto)*numeroVertici perche' verticiOggetto e' un puntatore a float e non piu un array di float

        //questo buffer invece contiene la sequenza di vertici da renderizzare
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indiciOggetto)*numeroIndici, indiciOggetto, GL_STREAM_DRAW);

        //l'array di vertici seguendo il buffer di indici andrà a raccogliere i vertici giusti volta per volta
        // attributi riguardanti la posizione, i primi 3 di ogni riga
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // attributi riguardanti le coordinate della texture rispetto ai vertici, gli ultimi 2 di ogni riga
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }
}
void Oggetto::setAngolo(float alpha){
    this->angolo=alpha;
}
void Oggetto::setVelocita(float velocita){
    this->velocita=velocita;
}

//i get importanti
glm::vec3 Oggetto::getPosizione(){
    return this->posizione;
}
float Oggetto::getAngolo(){
    return angolo;
}
float Oggetto::getVelocita(){
    return velocita;
}
int Oggetto::getTTL(){
    return timeToLive;
}
float Oggetto::getScala(){
    return scala;
}

//controlli
bool Oggetto::issetProgrammaShader(){
    if(this->programmaShaderDaUtilizzare==0) return false;
    else return true;
}
bool Oggetto::issetVerticiOggetti(){
    if(this->verticiOggetto==NULL || this->indiciOggetto==NULL || this->numeroVertici==0) return false;
    else return true;
}
bool Oggetto::issetTexture(){
    if(this->texture==0) return false;
    else return true;
}

bool Oggetto::update(){
    //modifica la posizione basandosi sulla velocita e sull'angolo dell'oggetto in quel preciso istante
    posizione.x+=velocita*glm::cos(glm::radians(angolo));
    posizione.y+=velocita*glm::sin(glm::radians(angolo));

    //per spostare da una parte all'altra dello schermo l'oggetto quando va oltre il bordo della finestra
    if(posizione.x>1.0f || posizione.x<-1.0f){
        posizione.x*=-1;
    }
    if(posizione.y>1.0f || posizione.y<-1.0f){
        posizione.y*=-1;
    }

    //per contare il numero di frame di vita di un oggetto
    if(timeToLive<300)timeToLive++;
    //std::cout<<posizione.x<<" "<<posizione.y<<" "<<velocita<<" "<<angolo<<"\n";

    //trasformazione vertici

    //da spazio locale a spazio globale local->global
    matrixModelTrasformation = glm::translate(glm::mat4(1.0), posizione);
    matrixModelTrasformation = glm::rotate(matrixModelTrasformation,glm::radians(-90.0f+angolo), glm::vec3(0.0, 0.0, +1.0));
    matrixModelTrasformation = glm::scale(matrixModelTrasformation, glm::vec3(scala,scala, 0.0));

    //da spazio globale a inquadratura camera global->view
    matrixViewTrasformation = glm::mat4(1.0);//se abilitata non si vedono piu gli oggetti
    //matrixViewTrasformation= glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, -3.0f));

    //da inquadratura camera a spazio su schermo camera view->viewport ndf
    //non supportata/non funzionante
    matrixProjectionTrasformation = glm::mat4(1.0);
    //matrixProjectionTrasformation = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);
    //matrixProjectionTrasformation = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);
    return true;
}
bool Oggetto::collidesWith(Oggetto* altroOggetto){
    //e' necessario rimemorizzarli perche altrimenti non e' possibile capire come sono posizionati
    std::vector<glm::vec4> vettoreVerticiOggetto1;
    float massimo=100000.0f,minimo=-100000.0f;
    float W1max=minimo, W1min=massimo,H1max=minimo,H1min=massimo;
    std::vector<glm::vec4> vettoreVerticiOggetto2;
    float W2max=minimo, W2min=massimo,H2max=minimo,H2min=massimo;

    //per entrambi gli oggetti
    //inserisco i vertici all'interno di un vettore di vertici per comodita
    int i=0;
    while(i<this->numeroVertici){
        //std::cout<<this->verticiOggetto[i]<<" "<<this->verticiOggetto[i+1]<<" ";
        vettoreVerticiOggetto1.push_back(glm::vec4(this->verticiOggetto[i],this->verticiOggetto[i+1],this->verticiOggetto[i+2],1.0f));
        //std::cout<<vettoreVerticiOggetto1[i/5].x<<" "<<vettoreVerticiOggetto1[i/5].y<<"\n";
        i+=5;
    }
    i=0;
    while(i<this->numeroVertici){
        //std::cout<<altroOggetto->verticiOggetto[i]<<" "<<altroOggetto->verticiOggetto[i+1]<<" ";
        vettoreVerticiOggetto2.push_back(glm::vec4(altroOggetto->verticiOggetto[i],altroOggetto->verticiOggetto[i+1],altroOggetto->verticiOggetto[i+2],1.0f));
        //std::cout<<vettoreVerticiOggetto2[i/5].x<<" "<<vettoreVerticiOggetto2[i/5].y<<"\n";
        i+=5;
    }

    //std::cout<<"vertici oggetto1\n";
    //this->stampaVertici();

    //trasforli in base alla posizione (perchè vertici sopra riguardano lo spazio locale e non globale), trova massimo e minimo in altezza e larghezza
    for(i=0;i<vettoreVerticiOggetto1.size();i++){
        vettoreVerticiOggetto1[i] = this->matrixModelTrasformation*vettoreVerticiOggetto1[i];
        //std::cout<<vettoreVerticiOggetto1[i].x<<" "<<vettoreVerticiOggetto1[i].y<<"\n";
        if(W1min>vettoreVerticiOggetto1[i].x)W1min=vettoreVerticiOggetto1[i].x;
        if(W1max<vettoreVerticiOggetto1[i].x)W1max=vettoreVerticiOggetto1[i].x;
        if(H1min>vettoreVerticiOggetto1[i].y)H1min=vettoreVerticiOggetto1[i].y;
        if(H1max<vettoreVerticiOggetto1[i].y)H1max=vettoreVerticiOggetto1[i].y;
    }
    //std::cout<<W1min<<" "<<W1max<<" "<<H1min<<" "<<H1max<<"\n";
    //std::cout<<"vertici oggetto2\n";
    //altroOggetto->stampaVertici();
    for(i=0;i<vettoreVerticiOggetto2.size();i++){
        vettoreVerticiOggetto2[i] = altroOggetto->matrixModelTrasformation*vettoreVerticiOggetto2[i];
        //std::cout<<vettoreVerticiOggetto2[i].x<<" "<<vettoreVerticiOggetto2[i].y<<"\n";
        if(W2min>vettoreVerticiOggetto2[i].x) W2min=vettoreVerticiOggetto2[i].x;
        if(W2max<vettoreVerticiOggetto2[i].x) W2max=vettoreVerticiOggetto2[i].x;
        if(H2min>vettoreVerticiOggetto2[i].y) H2min=vettoreVerticiOggetto2[i].y;
        if(H2max<vettoreVerticiOggetto2[i].y) H2max=vettoreVerticiOggetto2[i].y;
    }
    //std::cout<<W2min<<" "<<W2max<<" "<<H2min<<" "<<H2max<<"\n";

    //confrontare i vari massimi per scoprire se ci sono intersezioni tra i due

    //un intersezione per essere completa (oggetti veramente sovrapposti) deve essere in entrambi gli assi, se fosse 3d in tutti e tre dovrebbero sovrapporsi
    //significa che per un relativo asse la fine di uno dei due oggetti si posizione all'interno dell'altro oggetto
    int intersezione=0;
    if(W2min>=W1min && W2min<=W1max)
        intersezione+=1;
    else if(W2max>=W1min && W2max<=W1max)
        intersezione+=1;
    else if(W1min>=W2min && W1min<=W2max)
        intersezione+=1;
    else if(W1max>=W2min && W1max<=W2max)
        intersezione+=1;
    else intersezione+=0;
    if(intersezione>0){
        std::cout<<intersezione<<" ";
    }

    if(H2min>=H1min && H2min<=H1max)
        intersezione+=1;
    else if(H2max>=H1min && H2max<=H1max)
        intersezione+=1;
    else if(H1min>=H2min && H1min<=H2max)
        intersezione+=1;
    else if(H1max>=H2min && H1max<=H2max)
        intersezione+=1;
    else intersezione+=0;
    if(intersezione>0){
        std::cout<<intersezione<<"\n";
    }

    if(intersezione==2) return true;
    else return false;
    //questo metodo fa un po' schifo perchè ricalcola ogni volta i vertici e li inserisce nei vettori,
    //l'ideale sarebbe salverli dentro la classe e calcolare solo il minimo necessario
}
bool Oggetto::render(){
    return render(matrixModelTrasformation,matrixViewTrasformation,matrixProjectionTrasformation);
}
bool Oggetto::render(glm::mat4 model,glm::mat4 view,glm::mat4 projection){
    if(!issetProgrammaShader() || !issetVerticiOggetti() || !issetTexture())
        return false;
    else{
        /*non va fatto se no pulisce il frame a ogni oggetto renderizzato anziche una volta sola per frame
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);*/

        //per renderizzare un oggetto è necessario

        //caricare i programmi shader relativi (programmi per GPU)
        glUseProgram(programmaShaderDaUtilizzare);

        //caricare le matrici di trasformazione aggiornate per il frame all'interno della gpu
        glUniformMatrix4fv(locationMatrixModelTrasformation, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(locationMatrixViewTrasformation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(locationMatrixProjectionTrasformation, 1, GL_FALSE, glm::value_ptr(projection));

        //caricare anche la texture
        //glUniform1i(locationTexture, 0);//apparentemente non funziona
        glUniform1i(glGetUniformLocation(programmaShaderDaUtilizzare, "ourTexture"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        //specificare l'array da renderizzare e infine eseguire l'operzione vera e propria di "disegno"
        glBindVertexArray(VAO);

        //glDrawArrays(GL_TRIANGLES, 0, 3);//alternativa senza EBO
        glDrawElements(GL_TRIANGLES, numeroVertici, GL_UNSIGNED_INT, 0);

        //come ultimissima cosa è bene fare un unbind dell'array di vertici
        glBindVertexArray(0);

        return true;
    }
}

void Oggetto::stampaVertici(){
    int i=0;
    while(i<this->numeroVertici){
    std::cout<<verticiOggetto[i]<<" "<<verticiOggetto[i+1]<<"\n";
    i+=5;
    }
}
Oggetto::~Oggetto(){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
/*
prove
glm::mat4 model;
model = glm::translate(model, posizione);
model = glm::rotate(model,glm::radians(-90.0f+angolo), glm::vec3(0.0, 0.0, +1.0));
model = glm::scale(model, glm::vec3(scala,scala, 0.0));
--model = glm::translate(model, cubePositions[i]);//esempio*/
/*glm::mat4 projection;
projection = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);//
projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);*/
/*glm::mat4 view;
view= glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));*/
