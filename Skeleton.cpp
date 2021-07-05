//=============================================================================================
// Mintaprogram: Zold haromszog. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Gyomber Peter
// Neptun : PIM313
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const char * const vertexSource = R"(
	#version 330
	precision highp float;	

	layout(location = 0) in vec2 vp;
	layout(location = 1) in vec2 vertexUV;

	out vec2 textureCoord;

	void main() {
		textureCoord = vertexUV;
		gl_Position = vec4(vp.x, vp.y, 0, 1);
	}
)";

const char * const fragmentSource = R"(
	#version 330
	precision highp float;
	
	in vec2 textureCoord;
	uniform vec3 color1;
	uniform vec3 color2;
	uniform int isPoint; 
	out vec4 outColor;

	vec4 GenerateTexture(vec3 color1, vec3 color2, vec2 uv) {
					if (uv.x > 0 && uv.x < 1 && uv.y>0.0 && uv.y < 1) {
						if (uv.x < 0.5) {
							return vec4(color1.x, color1.y, color1.z, 1);
						}
						return vec4(color2.x, color2.y, color2.z, 1);
					}
					return vec4(0.5, 0.5, 0.5, 1);
				}	

	void main() {
		if(isPoint == 1)
			outColor = GenerateTexture(color1, color2, textureCoord);
		else
			outColor = vec4(1, 1, 0, 1);
	}
)";

GPUProgram gpuProgram;

class Drawable {
protected:
	unsigned int vao, vbo[2];
public:
	Drawable() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(2, vbo);
	}

	void DrawPoints(std::vector<vec2> vertices, std::vector<vec2> uvs) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		
		glBufferData(GL_ARRAY_BUFFER, 	
			sizeof(vec2)*vertices.size(),  
			&vertices[0],	      	
			GL_STATIC_DRAW);	

		glEnableVertexAttribArray(0); 
		glVertexAttribPointer(0,      
			2, GL_FLOAT, GL_FALSE,
			0, NULL); 		     

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, 
			sizeof(vec2)*uvs.size(), 
			&uvs[0],	      	
			GL_STATIC_DRAW);	

		glEnableVertexAttribArray(1); 
		glVertexAttribPointer(1,       
			2, GL_FLOAT, GL_FALSE, 
			0, NULL); 		     

	}

	void DrawEdges(std::vector<vec2> edges) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, 	
			sizeof(vec2) * edges.size(), 
			&edges[0],	      	
			GL_STATIC_DRAW);	

		glEnableVertexAttribArray(0);  
		glVertexAttribPointer(0,       
			2, GL_FLOAT, GL_FALSE, 
			0, NULL); 		    
	}

	virtual void Draw() = 0;

	~Drawable() {
		glDeleteBuffers(1, &vao);
		glDeleteVertexArrays(1, &vbo[0]);
	}
};

class Graph : public Drawable{
	unsigned int n = 50;
	float dPref = 0.5f;
	std::vector<vec3> points;
	std::vector<int> edges;
	std::vector<vec3> v;
	std::vector<vec3> pointColors;
public:
	Graph(): Drawable() {
		for (int i = 0; i < n; i++) {
			points.push_back(ToHomogen(vec2(0, 0)));
			v.push_back(vec3(0, 0, 0));
		}
		Shufflepoints(points);
		int edgeCount = (int)((n * (n - 1) / 2) * 0.05f);
		while(edges.size() / 2 < edgeCount) {
			AddRandomEdge();
		}
		for (int i = 0; i < n; i++) {
			float r1 = (rand() % 100) / 100.0f;
			float g1 = (rand() % 100) / 100.0f;
			float b1 = (rand() % 100) / 100.0f;
			float r2 = (rand() % 100) / 100.0f;
			float g2 = (rand() % 100) / 100.0f;
			float b2 = (rand() % 100) / 100.0f;
			pointColors.push_back(vec3(r1, g1, b1));
			pointColors.push_back(vec3(r2, g2, b2));
		}
	}

	void Shufflepoints(std::vector <vec3>& garphPoints) {
		for (int i = 0; i < n; i++) {
			float x = (rand() % 2 == 0) ? (rand() % 100) / 10.0f : (-rand() % 100) / 10.0f;
			float y = (rand() % 2 == 0) ? (rand() % 100) / 10.0f : (-rand() % 100) / 10.0f;
			x = x / 5.0f;
			y = y / 5.0f;
			garphPoints[i] = ToHomogen(vec2(x, y));
		}
	}

	void AddRandomEdge() {
		int random1 = rand() % n;
		int random2 = rand() % n;
		if (random1 != random2) {
			edges.push_back(random1);
			edges.push_back(random2);
		}
	}

	vec3 Reflection(vec3 p, vec3 m) {
		float d = GetDistanceBetween(p, m);
		vec3 v = DirectionVector(p, m);
		return (p * coshf(2 * d) + v * sinhf(2 * d));
	}

	void MoveGraph(vec3 m1, vec3 m2) {
		for (int i = 0; i < n; i++) {
			vec3 p1 = Reflection(points[i], m1);
			points[i] = Reflection(p1, m2);
		}
	}

	void MoveOrigoTo(vec3 q) {
		vec3 origo = ToHomogen(vec2(0, 0));
		float m1Distance = GetDistanceBetween(origo, q) / 3;
		vec3 m1V = DirectionVector(origo, q);
		vec3 m1 = origo * coshf(m1Distance) + m1V * sinhf(m1Distance);
		vec3 p1 = Reflection(origo, m1);
		float m2Distance = GetDistanceBetween(p1, q) / 2;
		vec3 m2V = DirectionVector(p1, q);
		vec3 m2 = p1 * coshf(m2Distance) + m2V * sinhf(m2Distance);
		MoveGraph(m1, m2);
	}

	bool IsEdgeBetween(int a, int b) {
		for (int i = 0; i < edges.size(); i+=2) {
			if((edges[i] == a && edges[i+1] == b) || (edges[i] == b && edges[i+1] == a))
				return true;
		}
		return false;
	}

	float Lorentz(vec3 a, vec3 b) {
		return  b.x * a.x + b.y * a.y - b.z * a.z;
	}

	float GetDistanceBetween(vec3 a, vec3 b) {
		if (a.x == b.x && a.y == b.y && a.z == b.z ) return 0;
		float l = Lorentz(a, b);
		if (-l < 1) l = -1;
		return acoshf(-l);
	}

	vec3 Fe(int a) {
		vec3 origo = ToHomogen(vec2(0, 0));
		float dFromOrigo = GetDistanceBetween(points[a], origo);
		vec3 fe = normalize(DirectionVector(points[a], origo)) * (powf(dFromOrigo, 2) * 0.6f);
		for (int i = 0; i < n; i++) {
			float d = GetDistanceBetween(points[a], points[i]);
			if (IsEdgeBetween(a, i)) {
				fe = fe + DirectionVector(points[a], points[i]) * (powf(d - dPref, 3) + 2.5f * (d - dPref)) * 0.05f;
			}
			else if (d != 0) {
				fe = fe + DirectionVector(points[a], points[i]) * (-0.02f / (d + 0.1f));
			}
		}
		return fe;	
	}

	vec3 DirectionVector(vec3 p, vec3 q) {
		float d = GetDistanceBetween(p, q);
		if (d == 0) return p;
		return (q - p * coshf(d)) / sinhf(d);
	}
	

	void ChangePosition(int a) {
		vec3 spEdge = points[a];
		vec3 spNoEdge = points[a];
		int mEdge = 1;
		int mNoEdge = 1;
		for (int i = 0; i < n; i++) {
			if (i != a) {
				if (IsEdgeBetween(a, i)) {
					float d = GetDistanceBetween(spEdge, points[i]);
					float r = d / (mEdge + 1);
					spEdge = spEdge * coshf(r) + sinhf(r) * DirectionVector(spEdge, points[i]);
					mEdge++;
				}
				else {
					float d = GetDistanceBetween(spNoEdge, points[i]);
					float r = d / (mNoEdge + 1);
					spNoEdge = spNoEdge * coshf(r) + sinhf(r) * DirectionVector(spNoEdge, points[i]);
					mNoEdge++;
				}
			}
		}
		float dDiff = GetDistanceBetween(spEdge, spNoEdge);
		float dOppDir = 0.005f / (dDiff + 0.5f);
		points[a] = spEdge * coshf(dOppDir) + sinhf(dOppDir) * (-DirectionVector(spEdge, spNoEdge));
		
	}
	
	void Draw() {
		std::vector<vec2> edgesToDraw;
		for (int i = 0; i < edges.size(); i++) {
			edgesToDraw.push_back(ToNormal(points[edges[i]]));
		}
		gpuProgram.setUniform(0, "isPoint");
		DrawEdges(edgesToDraw);
		glDrawArrays(GL_LINES, 0, edgesToDraw.size());

		gpuProgram.setUniform(1, "isPoint");
		std::vector<vec2> vertices;
		std::vector<vec2> uvs;
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < 20; j++) {
				float t = j * (2 * M_PI / 20);
	
				vec3 c = points[i] + vec3(cos(t), sin(t), 0);
				vec3 v = DirectionVector(points[i], ToHomogen(vec2(c.x, c.y)));
				float r = 0.05f;
				vec3 circlePoint = points[i] * coshf(r) + v * sinhf(r);
				vertices.push_back(ToNormal(circlePoint));

				uvs.push_back(vec2(cos(t) + 0.5, sin(t) + 0.5));
			}
		}
		DrawPoints(vertices, uvs);
		for (int i = 0; i < n; i++) {
			gpuProgram.setUniform(pointColors[i * 2], "color1");
			gpuProgram.setUniform(pointColors[i * 2 + 1], "color2");
			glDrawArrays(GL_TRIANGLE_FAN, i * 20, 20);
		}
	}

	vec3 ToHomogen(vec2 a) {
		return vec3(a.x, a.y, sqrtf(a.x * a.x + a.y * a.y + 1));
	} 
	vec2 ToNormal(vec3 a) {
		return vec2(a.x / a.z, a.y / a.z);
	}

	void Heuristics() {
		Shufflepoints(points);
		for (int i = 0; i < 1000; i++) {
			ChangePosition(i % n);
		}
		for (int i = 0; i < 5; i++) {
			Simulate(true);
		}
	}

	void Simulate(bool canMove) {	
		if (!canMove) {
			CorrigatePoints();
			return;
		}
		float deltaT = 0.05f;
		float m = 0.069f;
		for (int i = 0; i < n; i++) {
			vec3 fe = Fe(i);		
			v[i] = v[i] + fe / m * deltaT;
			v[i] = v[i] * 0.85f;
			float deltaS = length(v[i]) * deltaT;
			vec3 c = points[i] + v[i];
			vec3 iv = DirectionVector(points[i], ToHomogen(vec2(c.x, c.y)));

			points[i] = points[i] * coshf(deltaS) + sinhf(deltaS) * normalize(iv);		
		}
	}

	void CorrigatePoints() {
		for (int i = 0; i < n; i++) {
			points[i] = ToHomogen(vec2(points[i].x, points[i].y));
		}
	}
};

Graph* graph;

void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	glLineWidth(1.5f);	

	graph = new Graph();
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

bool simulate = false;

void onDisplay() {
	glClearColor(0, 0, 0, 0); 
	glClear(GL_COLOR_BUFFER_BIT); 
	
	graph->Draw();
	graph->Simulate(simulate);
	glutSwapBuffers(); 
}



void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == ' ') {
		graph->Heuristics();
		simulate = true;
		glutPostRedisplay(); 
	}	
}

void onKeyboardUp(unsigned char key, int pX, int pY) {}

vec2 mouseClick;
void onMouseMotion(int pX, int pY) {
	float cX = 2.0f * pX / windowWidth - 1;	
	float cY = 1.0f - 2.0f * pY / windowHeight;
	float x = (cX - mouseClick.x) / 25;
	float y = (cY - mouseClick.y) / 25;
	vec3 inv = vec3(x, y, 1) / sqrtf(1 - x * x - y * y);
	simulate = false;
	graph->MoveOrigoTo(inv);
}

void onMouse(int button, int state, int pX, int pY) { 
	float cX = 2.0f * pX / windowWidth - 1;
	float cY = 1.0f - 2.0f * pY / windowHeight;

	mouseClick = vec2(cX, cY);
}
void onIdle() {
	glutPostRedisplay();
}

