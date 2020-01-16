#ifndef OGGETTO_H
#define OGGETTO_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

class Oggetto{
    protected:
        float *verticiOggetto;
        glm::mat4 matrixModelTrasformation;
        glm::mat4 matrixViewTrasformation;
        glm::mat4 matrixProjectionTrasformation;

    private:
        int numeroVertici;
        unsigned int *indiciOggetto;

        unsigned int VBO;
        unsigned int EBO;
        unsigned int VAO;
        unsigned int texture;
        int programmaShaderDaUtilizzare;

        unsigned int locationMatrixModelTrasformation;
        unsigned int locationMatrixProjectionTrasformation;
        unsigned int locationMatrixViewTrasformation;
        unsigned int locationTexture;

        glm::vec3 posizione;
        float scala;
        float angolo;
        float velocita;

        int timeToLive;

    public:

        Oggetto();
        Oggetto(int programmaShader);
        Oggetto(float *verticiOggetto,int numeroVertici,unsigned int *indiciOggetto, int numeroIndici);
        Oggetto(int programmaShader, float *verticiOggetto,int numeroVertici,unsigned int *indiciOggetto, int numeroIndici, float scalaOggetto);
        Oggetto(int programmaShader, float *verticiOggetto,int numeroVertici,unsigned int *indiciOggetto, int numeroIndici, float scalaOggetto,const char *percorsoFileTexture);

        void setProgrammaShader(int programmaShader);
        void setVerticiOggetto(float *verticiOggetto,int numeroVertici,unsigned int *indiciOggetto,int numeroIndici);
        bool setTexture(const char *percorsoFileTexture);
        void setAngolo(float alpha);
        void setVelocita(float velocita);
        void setPosizione(glm::vec3 nuovo);

        float getAngolo();
        float getVelocita();
        glm::vec3 getPosizione();
        int getTTL();
        float getScala();

        bool issetProgrammaShader();
        bool issetVerticiOggetti();
        bool issetTexture();

        bool update();
        bool collidesWith(Oggetto* altroOggetto);

        bool render();
        bool render(glm::mat4 model,glm::mat4 view,glm::mat4 projection);

        void stampaVertici();
        ~Oggetto();
};

#endif // OGGETTO_H
