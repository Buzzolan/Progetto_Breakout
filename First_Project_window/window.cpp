#include <glad/glad.h>
#include <GLFW/glfw3.h>//per gestire finestre etc..
#include<iostream>
using namespace std;


//per shader semplici posso passarli sottoforma di stringa
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"//variabile di output con key word 'out'
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"//ad ogni frammento genere un colore fissato
//variabile di 4 elementi colore e livello di trasparenza
"}\n\0";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);// callback ridimensionamento finestra
void processInput(GLFWwindow* window);

int main() {
	float red;
	//codice standard per inizializzare il progetto
	glfwInit();//inizializzo glfw
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);// configuro glfw settando la versione
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);// definisco la versione opengl da usare, cioè la 3.3.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);// tipo di profilo (core)= nuova versione

	//creo l'oggetto finestra e richiede la dimensione di essa, il terzo elemento è il nome delle finestra
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);

	if (window == NULL)// gestisco se la finestra non è stata creata
	{
		cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);// il nostro contesto farà riferimento a quella finestra

	//inizializzo GLAD prima di chiamare funzioni openGl
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		/*passiamo GLAD alla funzione per caricare l'indirizzo dei puntatoria alla funzione 
		OpenGL che è specifico del SO. GLFW ci fornisce ProcAdress che definisce
		la funzione corretta in base al SO per cui stiamo compilando*/
	{
		cout << "Failed to initialize GLAD\n";
		return -1;
	}
	// primi due parametri è posizione angolo inferiore sx della finestra
	//stesse dim della finestra, setta il viewport
	glViewport(0, 0, 800, 600);

	// chiamo la funzione su ogni ridemensionamento della finestra
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	
	//definsico vertexshader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);// creo vertex shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // associo il vertex shader al sorgente
	glCompileShader(vertexShader);//compila a run time il vertex shader

	//definisco frament shader
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//ore ho i due shader compilati e devo lincarli al programma:
	//creo shader program che mette inseieme più shader e poi viene collegato al resto del programma
	// l'output di uno shader è l'input per quello successivo

	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();//creo il programa

	glAttachShader(shaderProgram, vertexShader);//attacco i vari shader
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);//faccio il link tra gli shader attaccati

	

	//ora devo cancellare gli oggetti shader
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		-0.5f, -0.5f, 0.0f, // left  
		 0.5f, -0.5f, 0.0f, // right 
		 0.0f,  0.5f, 0.0f  // top   
	};

	//gestione della memoria da CPU a GPU, grazie ad opengl
	unsigned int VBO, VAO;// il buffer ha id unico, puntatore al buffer
	glGenVertexArrays(1, &VAO);// genero VAO
	glGenBuffers(1, &VBO);// genera vbo

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	//faccio binding, associo al vbo una porzione
	//di memoria che definisco essere buffer di array(più comodo per vertici)
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);// dico dimensione della memoria da allocare e li passo la struttura
	//va a copiare il contenuto della memoria principale dove viene allocata la memoria per i vertici e lo copia nella gpu


	//devo lincare gli attributi dei vertici(pos, color, etc..)
	//il vertex shader non sa come prendere i dati dal vertex buffer object
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	/* il primo parametro specifica la posizione del primo vertice nel buffer(NB:location=0)
	   il seconod specifica la dimensione dell'attributo del vertice, è vec3 quindi 3 valori
	   il terzo specifica il tipo di dati(un vec in glsl è float)
	   il quarto se vogliamo che il dato sia normalizzato vedi pag 34 del libro
	   il quinto è conosciuto come lo stride e ci dice lo spazi tra vertici consecutivi
	   il sesto è l'offset di dove inizia nel buffer
	*/
	glEnableVertexAttribArray(0);// abilito l'array del buffer

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);


	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
																
    // creo render loop
	while (!glfwWindowShouldClose(window))// controllo che la finestra non sia stata chiusa ogni volta che inizio il loop
	{	
		//render loop
		processInput(window);//close window

		//renderig commands
		//red = (1 + sin(glfwGetTime())) / 2.0;
		glClearColor(0.2, 0.2f, 0.2f, 1.0f);//rosso, verde, blu
		glClear(GL_COLOR_BUFFER_BIT);// aggiorno buffer del colore

		glUseProgram(shaderProgram); //lo vado ad attivare
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		// check and call events and swap the buffer
		glfwSwapBuffers(window);// mando nella finestra quello che nel buffer
		//ho doppio buffer e quindi faccio swap
		glfwPollEvents();// intercetto eventi, es: ridimensione finestra, tastiere, movimenti mouse
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate();// pulisco/cancello tutte le risorse allocate per GLFW
	return 0;

}

void processInput(GLFWwindow* window)// chiudo finestra se premo esc
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)// aggiorno viewport
{
	glViewport(0, 0, width, height);
}