
#define PI 3.14159265
#define DEG90 90.0f
#define DEG180 180.0f
#define DEG270 270.0f
#define DEG360 360.0f
#define RAD90 1.57079633f
#define RAD180 PI
#define RAD270 5.3796286f
#define RAD360 6.2831853f


double findSphereVolume(float r);

double findDist(Eigen::Vector3f p1, Eigen::Vector3f p2);

Eigen::Vector3f findCrossProduct(Eigen::Vector3f b, Eigen::Vector3f a);

double findMagVec(Eigen::Vector3f x);

double findAngDotProductR(Eigen::Vector3f a, Eigen::Vector3f b);

double findAngDotProductD(Eigen::Vector3f a, Eigen::Vector3f b);

double findDot(Eigen::Vector3f a, Eigen::Vector3f b);

Eigen::Vector3f findUnit(Eigen::Vector3f vec);

double solveElasticCollision1D(double m1, double m2, double v1, double v2, double e);

int angleAcuteR(float ang);

int angleAcuteD(float ang);

int findSignf(double x);