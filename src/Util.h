#include <string>
#include <Eigen/Dense>

std::string strToUpper(const char* from);
std::string& strToUpper(std::string& str);
std::string strToUpper(const std::string& str);

Eigen::Affine3f& roundMatrix(Eigen::Affine3f& mat);
Eigen::Affine3f roundMatrix(const Eigen::Affine3f& mat);

Eigen::Vector3f roundVector(const Eigen::Vector3f& vec);
Eigen::Vector2f roundVector(const Eigen::Vector2f& vec);

float round(float num);

std::string getCanonicalPath(const std::string& str);