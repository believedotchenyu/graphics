#include "bezier.hpp"
#include <Eigen/Core>
#include <Eigen/Dense>
#include <assert.h>

using namespace std;
using namespace Eigen;

void Bezier::append(const Point& p)
{
    assert(dcmp(p.y) == 0);
    points.push_back(p);
}

void Bezier::maintain(double scale)
{
    for(int i = 0; i < (int)points.size(); i ++)
        points[i] = points[i]*scale;
    double zmin = points[0].z, zmax = points[0].z;
    for(int i = 0; i < (int)points.size(); i ++)
    {
        zmin = min(zmin, points[i].z);
        zmax = max(zmax, points[i].z);
    }
    box_center = Point(0, 0, (zmax+zmin)/2);
    box_radius2 = 0;
    for(double t = 0; t <= 1; t += 1e-2)
        box_radius2 = max(box_radius2, Length2(P(t)-box_center));
    cout << "==========Bezier===========" << endl;
    box_center.print();
    cout << box_radius2 << endl;
}

Point Bezier::P(double t) const
{
    // assert(points.size() == 3);
    // return (1-t)*(1-t)*points[0]+2*t*(1-t)*points[1]+t*t*points[2];
    DMatrix<Point> mat(points.size(), points.size());
    for(int i = 0; i < (int)points.size(); i ++)
        mat[points.size()-1][i] = points[i];
    for(int i = points.size()-2; i >= 0; i --)
        for(int j = 0; j <= i; j ++)
            mat[i][j] = mat[i+1][j] + (mat[i+1][j+1]-mat[i+1][j])*t;
    return mat[0][0];
}

Point Bezier::P(double t, double theta) const
{
    Point p = P(t);
    return Point(p.x*cos(theta), p.x*sin(theta), p.z);
}

Vector Bezier::Pt(double t) const
{
    if (points.size() == 3)
    {
        // B(t) = (1-t)^2*P0+2*t*(1-t)*P1+t^2*P2
        return -2*(1-t)*points[0]+2*(1-2*t)*points[1]+2*t*points[2];
    }
    if (points.size() == 4)
    {
        // B(t) = (1-t)^3*P0+3*t*(1-t)^2*P1+3*t^2*(1-t)*P2+t^3*P3;
        return -3*(1-t)*(1-t)*points[0] + 3*((1-t)*(1-t)-t*2*(1-t))*points[1] + 3*(2*t*(1-t)-t*t)*points[2] + 3*t*t*points[3];
    }
    assert(false);
    return Vector();
}

Vector Bezier::N(double t, double theta) const
{
    Vector pt = Pt(t);
    Vector n(pt.z*cos(theta), pt.z*sin(theta), -pt.x);
    return Normalize(n);
}

cv::Mat Bezier::draw(int size) const
{
    cv::Mat img(size, size, CV_8UC3, cv::Scalar(255, 255, 255));
    for(double t = 0; t < 1.0; t += 0.5/size)
    {
        Point p = P(t);
        int x = -p.z*size/2+size/2;
        int y = p.x*size/2+size/2;
        img.at<cv::Vec3b>(x, y) = cv::Vec3b(255, 0, 0);
    }
    return img;
}

CollideInfo Bezier::collide(const Ray& ray, bool rayin) const
{
    Vector l = box_center - ray.s;
    double tp = Dot(l, ray.d);
    double d2 = Length2(l) - tp*tp;
    if (dcmp(d2 - box_radius2) > 0) return NoCollide;

    auto F = [&ray, this](const Matrix<double,3,1>& in) {
        double x = in[0], theta = in[1], t = in[2];
        Point p = P(x, theta);
        Matrix<double,3,1> ret;
        ret[0] = ray.s.x+ray.d.x*t-p.x;
        ret[1] = ray.s.y+ray.d.y*t-p.y;
        ret[2] = ray.s.z+ray.d.z*t-p.z;
        return ret;
    };
    auto Ft = [&ray, this](const Matrix<double,3,1>& in) {
        double x = in[0], theta = in[1]; // t = in[2];
        Point p = P(x);
        Vector pt = Pt(x);
        Matrix<double,3,3> ret;
        ret(0,0)=-pt.x*cos(theta); ret(0,1)=p.x*sin(theta); ret(0,2)=ray.d.x;
        ret(1,0)=-pt.x*sin(theta); ret(1,1)=-p.x*cos(theta); ret(1,2)=ray.d.y;
        ret(2,0)=-pt.z; ret(2,1)=0; ret(2,2)=ray.d.z;
        return ret;
    };

    Matrix<double,3,1> solution;
    double R = 1e10;

    for(int xtimes = 0; xtimes < 5 && dcmp(R) > 0; xtimes ++)
        for(int thetatimes = 0; thetatimes < 5 && dcmp(R) > 0; thetatimes ++)
        {
            Matrix<double,3,1> x;
            x[0] = 0.5/5.0+xtimes/5.0;
            x[1] = PI*2*0.5/5.0+thetatimes*PI*2/5.0;
            Point p = P(x[0], x[1]);
            Vector l = p - ray.s;
            x[2] = Dot(l, ray.d);
            // x[0] = RAND();
            // x[1] = RAND()*PI*2;
            // x[2] = RAND()*Length(ray.s);

            for(int iter = 8; iter && dcmp(R) > 0; iter --)
            {
                auto tmp = F(x);
                double module = sqrt(tmp[0]*tmp[0]+tmp[1]*tmp[1]+tmp[2]*tmp[2]);
                Vector n = N(x[0], x[1]);
                if (rayin) n = -n;
                if (dcmp(x[0])>=0 && dcmp(x[0]-1)<=0 && x[2]>1e-2 && dcmp(module) == 0 && dcmp(Dot(n, ray.d)) < 0 && R > module)
                {
                    R = module;
                    solution = x;
                }
                x = x-Ft(x).inverse().eval()*tmp;
            }
        }

    if (dcmp(R) > 0) return NoCollide;
    Point p = P(solution[0], solution[1]);
    Vector n = N(solution[0], solution[1]);
    if (rayin) n = -n;
    double t = Length(p-ray.s);
    if (dcmp(Length2(ray.s+ray.d*t-p)) != 0) cout << Length2(ray.s+ray.d*t-p) << endl;
    assert(dcmp(Length2(ray.s+ray.d*t-p)) == 0);
    return (CollideInfo){t, p, n, ray.d};
}

Point Bezier::center() const
{
    return box_center;
}