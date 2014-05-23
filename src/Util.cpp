#include "Util.h"
#include <boost/filesystem.hpp>

std::string strToUpper(const char* from)
{
	std::string str(from);
	for(unsigned int i = 0; i < str.size(); i++)
		str[i] = toupper(from[i]);
	return str;
}

std::string& strToUpper(std::string& str)
{
	for(unsigned int i = 0; i < str.size(); i++)
		str[i] = toupper(str[i]);

	return str;
}

std::string strToUpper(const std::string& str)
{
	return strToUpper(str.c_str());
}


#if _MSC_VER < 1800
float round(float num)
{
	return (float)((int)(num + 0.5f));
}
#endif

Eigen::Affine3f& roundMatrix(Eigen::Affine3f& mat)
{
	mat.translation()[0] = round(mat.translation()[0]);
	mat.translation()[1] = round(mat.translation()[1]);
	return mat;
}

Eigen::Affine3f roundMatrix(const Eigen::Affine3f& mat)
{
	Eigen::Affine3f ret = mat;
	roundMatrix(ret);
	return ret;
}

Eigen::Vector3f roundVector(const Eigen::Vector3f& vec)
{
	Eigen::Vector3f ret = vec;
	ret[0] = round(ret[0]);
	ret[1] = round(ret[1]);
	ret[2] = round(ret[2]);
	return ret;
}

Eigen::Vector2f roundVector(const Eigen::Vector2f& vec)
{
	Eigen::Vector2f ret = vec;
	ret[0] = round(ret[0]);
	ret[1] = round(ret[1]);
	return ret;
}

std::string getCanonicalPath(const std::string& path)
{
	if(boost::filesystem::exists(path))
		return boost::filesystem::canonical(path).generic_string();
	else
		return "";
}
