#pragma once
#pragma once
#include "opencv2\opencv.hpp"

using namespace cv;
using namespace std;


class new_carprosty
{
private:
	// wsp�rzedne �rodka, wymiary i kat
	float x, y, z, lx, ly, lz, h, a, b, at, bt, sg, hf, hb, angle;
	//float h = float(2.0 / 3.0);
	//macierze z modelem obiektu; model podstawowy, bie��cy (po transformacji) i po rzutowaniu na p�aszczyzn� obrazu
	Mat base_model3D, model3D, model2D;
	//macierz przekszta�cenia modelu podstawowego do bie��cego
	Mat T;

	//tworzy model bazowy na podstawie parametr�w oraz przekszta�ca go do modelu bie��cego
	void create_model()
	{
		//wygodnie kolejne punkty podawa� wierszami
		//od razu wsp�rz�dne uog�lnione
		//model podstawowy, �rodek podstawy w punkcie 0,0,0
		//a wi�c punkty po p� d�ugo�ci w ka�dym wymiarze na minusie i plusie


		base_model3D = (Mat_<float>(8, 4) <<
			//podstawa
			-lx * .5, -ly * .5, 0, 1,
			lx * .5, -ly * .5, 0, 1,
			lx * .5, ly * .5, 0, 1,
			-lx * .5, ly * .5, 0, 1,

			//g�ra
			-lx * .5, -ly * .5, lz, 1,
			lx * .5, -ly * .5, lz, 1,
			lx * .5, ly * .5, lz, 1,
			-lx * .5, ly * .5, lz, 1);


		//transpozycja modelu, by wsp�rz�dne by�y pionowo - wygodniej do p�niejszych oblicze�
		base_model3D = base_model3D.t();

		//wyznaczenie k�ta, cosinusa i sinusa - liczymy to raz zamiast kilka razy - szybciej
		float a = angle * 0.0174532925;
		float c = cos(a);
		float s = sin(a);

		//obr�t wok� osi z i przesuniecie - jedna macierz transformacji
		T = (Mat_<float>(4, 4) <<
			c, -s, 0, x,
			s, c, 0, y,
			0, 0, 1, z,
			0, 0, 0, 1);

		//przekszta�cenie modelu bazowego do bie��cego - wsp�rz�dne ju� s� jednorodne, wi�c po prostu mno�enie przez T
		model3D = T * base_model3D;
	}
	//powy�sze zmienne oraz metoda create s� prywatne - nie mamy do nich dost�pu z zewn�trz
	//aby czego� przypadkiem nie popsu�, np. zmienimy x i my�limy, �e ca�y model si� zmieni�.
	//a tak nie jest

public:
	//konstruktor - przyjmuje domy�lne parametry prostopad�o�cianu
	new_carprosty(float lx = 10, float ly = 10, float lz = 10, float x = 0, float y = 0, float angle = 0) {

		this->x = x;
		this->y = y;
		this->angle = angle;
		this->lx = lx;
		this->ly = ly;
		this->lz = lz;
		
		create_model();
	}

	//zmienia parametry modelu
	//metoda nie jest optymalna, bo za ka�dym razem tworzy model od nowa, podczas gdy w przypadku przesuni�cia po osi x
	//wystarczy zmodyfikowa� jedn� pozycj� w macierzy T i wykona� mno�enie istniej�cego modelu
	//jednak algorytm PSO i tak zmienia najcz�ciej wszystkie parametry modelu, wi�c tak jest do�� wygodnie
	//cho� szybciej by�oby zaktualizowa�


	//rzutowanie modelu bie��cego 3D na p�aszczyzn� obrazu (do: model2D)
	void project(Mat& rot, Mat& trans, Mat& cam, Mat& dist)
	{
		//rzutowanie musi by� we wsp�rz�dnych "normalnych", nie jednorodnych
		//wi�c obcinamy w locie ostatni wiersz (od 0 do 3 WY��cznie, czyli 0, 1, 2)
		projectPoints(model3D.rowRange(0, 3), rot, trans, cam, dist, model2D); //model2D to dwukana�owa macierz wierszowa
		model2D = model2D.reshape(1).t(); //przekszta�camy do jednokana�owej macierzy kolumnowej
		//mog�aby by� i wierszowa, wszystko jedno, ale �eby si� nie myli�o z modelem3D, kt�ry jest kolumnowy
	}

	//rysuje model2D na obrazie
	void draw(Mat& img, int xs = 0, int ys = 0)
	{
		//metoda zak�ada, �e model2D istnieje - jak b�dzie pusty, wyrzuci b��d
		//oczywi�cie uk�ad �cian jest dla prostopad�o�cianu - dla innego kszta�tu trzeba zmodyfikowa�
		vector<Point> face;
		vector<vector<Point>> faces;

		//sciana pozioma dolna
		/*face.push_back((Point)model2D.col(0));
		face.push_back((Point)model2D.col(1));
		face.push_back((Point)model2D.col(2));
		face.push_back((Point)model2D.col(3));
		faces.push_back(face);*/

		//sciana pozioma gora
		face.clear();
		face.push_back((Point)model2D.col(4) + Point(xs,ys));
		face.push_back((Point)model2D.col(5) + Point(xs,ys));
		face.push_back((Point)model2D.col(6) + Point(xs,ys));
		face.push_back((Point)model2D.col(7) + Point(xs,ys));
		faces.push_back(face);

		//sciana pionowa przod
		face.clear();
		face.push_back((Point)model2D.col(1) + Point(xs,ys));
		face.push_back((Point)model2D.col(2) + Point(xs,ys));
		face.push_back((Point)model2D.col(6) + Point(xs,ys));
		face.push_back((Point)model2D.col(5) + Point(xs,ys));
		faces.push_back(face);

		//sciana pionowa tyl
		face.clear();
		face.push_back((Point)model2D.col(0) + Point(xs,ys));
		face.push_back((Point)model2D.col(4) + Point(xs,ys));
		face.push_back((Point)model2D.col(7) + Point(xs,ys));
		face.push_back((Point)model2D.col(3) + Point(xs,ys));
		faces.push_back(face);

		//sciana pionowa bok1
		face.clear();
		face.push_back((Point)model2D.col(2) + Point(xs,ys));
		face.push_back((Point)model2D.col(3) + Point(xs,ys));
		face.push_back((Point)model2D.col(7) + Point(xs,ys));
		face.push_back((Point)model2D.col(6) + Point(xs,ys));
		faces.push_back(face);

		//sciana pionowa bok2
		face.clear();
		face.push_back((Point)model2D.col(0) + Point(xs,ys));
		face.push_back((Point)model2D.col(1) + Point(xs,ys));
		face.push_back((Point)model2D.col(5) + Point(xs,ys));
		face.push_back((Point)model2D.col(4) + Point(xs,ys));
		faces.push_back(face);

	

		//maj�c zbudowane wszystkie �ciany, mo�emy je narysowa� po kolei
		for (int i = 0; i < faces.size(); i++)
			fillConvexPoly(img, faces[i], Scalar(255)); //rysuje jeden wielok�t wypuk�y na raz i jest szybsza od fillPoly




	}

	//rysuje kraw�dzie samochodu
	void drawEdges(Mat& img)
	{
		int g = 1;
		//podstawa
		line(img, (Point)model2D.col(0), (Point)model2D.col(1), Scalar(200), g, LineTypes::LINE_AA);
		line(img, (Point)model2D.col(1), (Point)model2D.col(2), Scalar(200), g, LineTypes::LINE_AA);
		line(img, (Point)model2D.col(2), (Point)model2D.col(3), Scalar(200), g, LineTypes::LINE_AA);
		line(img, (Point)model2D.col(3), (Point)model2D.col(0), Scalar(200), g, LineTypes::LINE_AA);

		//pionowe
		line(img, (Point)model2D.col(1), (Point)model2D.col(5), Scalar(100), g, LineTypes::LINE_AA);
		line(img, (Point)model2D.col(2), (Point)model2D.col(6), Scalar(100), g, LineTypes::LINE_AA);
		line(img, (Point)model2D.col(3), (Point)model2D.col(7), Scalar(100), g, LineTypes::LINE_AA);
		line(img, (Point)model2D.col(0), (Point)model2D.col(4), Scalar(100), g, LineTypes::LINE_AA);

		//poziome g�ra
		line(img, (Point)model2D.col(5), (Point)model2D.col(4), Scalar(100, 0, 100), g, LineTypes::LINE_AA);
		line(img, (Point)model2D.col(6), (Point)model2D.col(5), Scalar(100, 0, 100), g, LineTypes::LINE_AA);
		line(img, (Point)model2D.col(7), (Point)model2D.col(6), Scalar(100, 0, 100), g, LineTypes::LINE_AA);
		line(img, (Point)model2D.col(4), (Point)model2D.col(7), Scalar(100, 0, 100), g, LineTypes::LINE_AA);

	}

};

