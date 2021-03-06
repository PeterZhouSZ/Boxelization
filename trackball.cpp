#include <windows.h>
#include <gl\GL.h>
#include <Qt>
#include "trackball.h"

TrackBall::TrackBall()
{
	last_point_2D = Vector2D(0.0, 0.0);
	last_point_3D = Vector3D(0.0, 0.0, 0.0);
	zoom_center = Vector3D(0.0, 0.0, 0.0);
	rotate_center = Vector3D(0.0, 0.0, 0.0);
}

TrackBall::TrackBall(int width, int height)
{
	Width = width; Height = height;
	double radius = 2 * sqrt((double)(Width*Width)+(double)(Height*Height));
	adjustWidth = 1 / radius;
	adjustHeight = 1 / radius;

	last_point_2D = Vector2D(0.0, 0.0);
	last_point_3D = Vector3D(0.0, 0.0, 0.0);
	zoom_center = Vector3D(0.0, 0.0, 0.0);
	rotate_center = Vector3D(0.0, 0.0, 0.0);
}

TrackBall::~TrackBall()
{
	
}

void TrackBall::mapToSphere(const Vector2D& v2d, Vector3D& v3d)
{
// 	double x = v2d.x * adjustWidth - 1.0;
// 	double y = 1.0 - v2d.y * adjustHeight;
	double x = (v2d.x - Width/2) * adjustWidth;
	double y = (Height/2 - v2d.y) * adjustHeight;

	double x2y2 = x*x+y*y;

	if (x2y2 < 1.0)
	{
		v3d.x = x;
		v3d.y = y;
		v3d.z = sqrt(1.0 - x2y2);
	}
	else
	{
		double norm = 1.0 / sqrt(x2y2);
		v3d.x = x * norm;
		v3d.y = y * norm;
		v3d.z = 0;
	}
}

void TrackBall::init()
{
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			MVmatrix[i * 4 + j] = i == j ? 1 : 0;
	zoomRadius = 0.01;
}

void TrackBall::apply()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(MVmatrix);
}

void TrackBall::reAdjustTrackBall(double width, double height)
{
	Width = width; Height = height;
	double radius = sqrt((double)(Width*Width)+(double)(Height*Height));
	adjustWidth = 1 / radius;
	adjustHeight = 1 / radius;
}

void TrackBall::mouseMove(int button, Vector2D v2d)
{
	if (button == Qt::LeftButton)
	{
		Vector3D new_point_3D;
		mapToSphere(v2d, new_point_3D);

		Vector3D axis = last_point_3D ^ new_point_3D;
		if (axis.length() < 1e-7) axis = Vector3D(0, 0, 0);
		else axis.normalize();

		Vector3D dist = last_point_3D - new_point_3D;
		double sinangle = 0.5*dist.length();
		double angle = asin(sinangle) / PI * 180.0;

		translate(rotate_center * (-1.0));
		rotate(axis, 8 * angle);
		translate(rotate_center);

		last_point_3D = new_point_3D;
		last_point_2D = v2d;
	}
	else if (button == Qt::RightButton)
	{
		double dist = exp((last_point_2D.y - v2d.y)*zoomRadius);
		Vector3D scaleFactor(dist, dist, dist);

		translate(zoom_center * (-1.0));
		scale(scaleFactor);
		translate(zoom_center);

		rotate_center -= zoom_center;
		rotate_center *= dist;
		rotate_center += zoom_center;

		last_point_2D = v2d;
	}
	else if (button == Qt::MidButton)
	{
		double dx = v2d.x - last_point_2D.x;
		double dy = v2d.y - last_point_2D.y;

		translate(Vector3D(2.0 * dx / Width, -2.0 * dy / Width, 0));
		rotate_center += Vector3D(2.0 * dx / Width, -2.0 * dy / Width, 0);

		last_point_2D = v2d;
	}
}

void TrackBall::mouseClick(int button, int buttonState, Vector2D v2d, Vector3D v3d, bool updateCenter)
{
	if (button == Qt::LeftButton)
	{
		last_point_2D = v2d;
		mapToSphere(last_point_2D, last_point_3D);
		if (updateCenter) rotate_center = v3d;
	}
	else if (button == Qt::RightButton)
	{
		zoom_center = v3d;
		last_point_2D = v2d;
	}
	else if (button == Qt::MiddleButton) 
	{
		last_point_2D = v2d;
	}
}

void TrackBall::rotate(const Vector3D& axis, double angle)
{	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotated(angle, axis.x, axis.y, axis.z);
	glMultMatrixd(MVmatrix);
	glGetDoublev(GL_MODELVIEW_MATRIX, MVmatrix);
}

void TrackBall::translate(const Vector3D& shift)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(shift.x, shift.y, shift.z);
	glMultMatrixd(MVmatrix);
	glGetDoublev(GL_MODELVIEW_MATRIX, MVmatrix);
}

void TrackBall::scale(const Vector3D& scaleFactor)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScaled(scaleFactor.x, scaleFactor.y, scaleFactor.z);
	glMultMatrixd(MVmatrix);
	glGetDoublev(GL_MODELVIEW_MATRIX, MVmatrix);
}

// void TrackBall::MultipleMatrixVector(double *m, Vector3D &rotate_center)
// {
// 	Vector3D rc;
// 	rc.x = m[0] * rotate_center.x + m[4] * rotate_center.y + m[8] * rotate_center.z + m[12];
// 	rc.y = m[1] * rotate_center.x + m[5] * rotate_center.y + m[9] * rotate_center.z + m[13];
// 	rc.z = m[2] * rotate_center.x + m[6] * rotate_center.y + m[10] * rotate_center.z + m[14];
// 	rotate_center = rc;
// }