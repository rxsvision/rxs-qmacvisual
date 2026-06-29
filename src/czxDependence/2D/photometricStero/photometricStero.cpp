#include "photometricStero.h"
#include <Eigen/Sparse>
//#include"photometricStero_cu.h"

using namespace cv;
namespace czx
{
    std::vector<cv::Vec3f> convertSlantTiltArrayToLights(const std::vector<float>& slants, const std::vector<float>& tilts)
    {

        if (slants.size() != tilts.size())
            throw logic_error("倾斜角和俯仰角数目不一致");
        std::vector<cv::Vec3f> lights;
        size_t size = slants.size();

        for (size_t i = 0; i < size; ++i) {
            float slant = slants[i];
            float tilt = tilts[i];
            double alpha = slant * M_PI / 180.0; // Convert degrees to radians
            double beta = tilt * M_PI / 180.0;   // Convert degrees to radians
            double x = sin(alpha) * cos(beta);
            double y = sin(alpha) * sin(beta);
            double z = cos(alpha);
            lights.push_back(cv::Vec3f(x, y, z));
        }
        return lights;
    }

    PhotometricSteroCZX::PhotometricSteroCZX(const vector<string>& img_paths, const vector<Vec3f>& lights)
    {

        CzxTimer _(__func__);
        if (img_paths.size() != lights.size())
            throw logic_error("光线数目与图片数目不一致");
        if (img_paths.size() == 0)
            throw logic_error("没有图片");

        this->img_paths = img_paths;
        images.clear();
        for (auto& path : img_paths)
        {
            auto img = cv::imread(path, cv::IMREAD_GRAYSCALE);
            images.push_back(img);
        }

        //try
        //{
        //    photometric_stereo_GPU(images, lights, Normals, Pgrads, Qgrads, Albedo);
        //}
        //catch (const std::exception& e) {
        //    std::cerr << "Exception: " << e.what() << std::endl;
        //}
        //return;

        int height = images[0].rows;
        int width = images[0].cols;
        num_imamges = images.size();
        Mat Lights(num_imamges, 3, CV_32F);
        for (int i = 0; i < num_imamges; i++)
        {
            Lights.at<float>(i, 0) = lights[i][0];
            Lights.at<float>(i, 1) = lights[i][1];
            Lights.at<float>(i, 2) = lights[i][2];
        }

        cv::invert(Lights, LightsInv, cv::DECOMP_SVD);

        Normals = Mat(height, width, CV_32FC3, cv::Scalar::all(0));
        Pgrads = Mat(height, width, CV_32F, cv::Scalar::all(0));
        Qgrads = Mat(height, width, CV_32F, cv::Scalar::all(0));
        Albedo = Mat(height, width, CV_32F, cv::Scalar::all(0));



        /* estimate surface normals and p,q gradients */
        #pragma omp parallel for
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                vector<float> I(images.size());
                for (int i = 0; i < num_imamges; i++)
                {
                    I[i] = images[i].at<uchar>(Point(x, y));
                }

                cv::Mat n = LightsInv * cv::Mat(I);
                float p = sqrt(cv::Mat(n).dot(n));
                Albedo.at<float>(cv::Point(x, y)) = p / 255;
                if (p > 0) { n = n / p; }
                if (n.at<float>(2, 0) == 0) { n.at<float>(2, 0) = 1.0; }
                int legit = 1;
                /* avoid spikes ad edges */
                for (int i = 0; i < num_imamges; i++) {
                    legit *= images[i].at<uchar>(Point(x, y)) >= 0;
                }
                if (legit)//&&x>400&&x<width- 400 &y>400 &&y<height- 400)
                {
                    Normals.at<cv::Vec3f>(cv::Point(x, y)) = n;
                    Pgrads.at<float>(cv::Point(x, y)) = n.at<float>(0, 0) / n.at<float>(2, 0);
                    Qgrads.at<float>(cv::Point(x, y)) = n.at<float>(1, 0) / n.at<float>(2, 0);
                }
                else
                {
                    cv::Vec3f nullvec(0.0f, 0.0f, 1.0f);
                    Normals.at<cv::Vec3f>(cv::Point(x, y)) = nullvec;
                    Pgrads.at<float>(cv::Point(x, y)) = 0.0f;
                    Qgrads.at<float>(cv::Point(x, y)) = 0.0f;
                    Albedo.at<float>(cv::Point(x, y)) = 0.0f;

                }
            }
        }
    }

    PhotometricSteroCZX::PhotometricSteroCZX(const vector<string>& img_paths, const vector<float>& slants, const vector<float>& tilts) :PhotometricSteroCZX(img_paths, convertSlantTiltArrayToLights(slants, tilts))
    {
        //CzxTimer _(__func__);
        auto lights = convertSlantTiltArrayToLights(slants, tilts);
        for (const auto& normal : lights)
        {
            std::cout << "光线方向向量 (法向量): ("
                << normal[0] << ", "
                << normal[1] << ", "
                << normal[2] << ")" << std::endl;
        }

    }

    PhotometricSteroCZX::PhotometricSteroCZX(const vector<cv::Mat>& imgs, const vector<float>& slants, const vector<float>& tilts)
    {
        auto lights = convertSlantTiltArrayToLights(slants, tilts);
        images = imgs;
        int height = images[0].rows;
        int width = images[0].cols;
        num_imamges = images.size();
        Mat Lights(num_imamges, 3, CV_32F);
        for (int i = 0; i < num_imamges; i++)
        {
            Lights.at<float>(i, 0) = lights[i][0];
            Lights.at<float>(i, 1) = lights[i][1];
            Lights.at<float>(i, 2) = lights[i][2];
        }

        cv::invert(Lights, LightsInv, cv::DECOMP_SVD);

        Normals = Mat(height, width, CV_32FC3, cv::Scalar::all(0));
        Pgrads = Mat(height, width, CV_32F, cv::Scalar::all(0));
        Qgrads = Mat(height, width, CV_32F, cv::Scalar::all(0));
        Albedo = Mat(height, width, CV_32F, cv::Scalar::all(0));



        /* estimate surface normals and p,q gradients */
        #pragma omp parallel for
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                vector<float> I(images.size());
                for (int i = 0; i < num_imamges; i++)
                {
                    I[i] = images[i].at<uchar>(Point(x, y));
                }

                cv::Mat n = LightsInv * cv::Mat(I);
                float p = sqrt(cv::Mat(n).dot(n));
                Albedo.at<float>(cv::Point(x, y)) = p / 255;
                if (p > 0) { n = n / p; }
                if (n.at<float>(2, 0) == 0) { n.at<float>(2, 0) = 1.0; }
                int legit = 1;
                /* avoid spikes ad edges */
                for (int i = 0; i < num_imamges; i++) {
                    legit *= images[i].at<uchar>(Point(x, y)) >= 0;
                }
                if (legit)//&&x>400&&x<width- 400 &y>400 &&y<height- 400)
                {
                    Normals.at<cv::Vec3f>(cv::Point(x, y)) = n;
                    Pgrads.at<float>(cv::Point(x, y)) = n.at<float>(0, 0) / n.at<float>(2, 0);
                    Qgrads.at<float>(cv::Point(x, y)) = n.at<float>(1, 0) / n.at<float>(2, 0);
                }
                else
                {
                    cv::Vec3f nullvec(0.0f, 0.0f, 1.0f);
                    Normals.at<cv::Vec3f>(cv::Point(x, y)) = nullvec;
                    Pgrads.at<float>(cv::Point(x, y)) = 0.0f;
                    Qgrads.at<float>(cv::Point(x, y)) = 0.0f;
                    Albedo.at<float>(cv::Point(x, y)) = 0.0f;

                }
            }
        }
    }


    cv::Mat PhotometricSteroCZX::globalHeights()
    {
        cv::Mat P(Pgrads.rows, Pgrads.cols, CV_32FC2, cv::Scalar::all(0));
        cv::Mat Q(Pgrads.rows, Pgrads.cols, CV_32FC2, cv::Scalar::all(0));
        cv::Mat Z(Pgrads.rows, Pgrads.cols, CV_32FC2, cv::Scalar::all(0));

        float lambda = 1.0f;
        float mu = 1.0f;

        cv::dft(Pgrads, P, cv::DFT_COMPLEX_OUTPUT);
        cv::dft(Qgrads, Q, cv::DFT_COMPLEX_OUTPUT);
        for (int i = 0; i < Pgrads.rows; i++) {
            for (int j = 0; j < Pgrads.cols; j++) {
                if (i != 0 || j != 0)
                {
                    float u = sin((float)(i * 2 * CV_PI / Pgrads.rows));
                    float v = sin((float)(j * 2 * CV_PI / Pgrads.cols));

                    float uv = pow(u, 2) + pow(v, 2);
                    float d = (1.0f + lambda) * uv + mu * pow(uv, 2);
                    Z.at<cv::Vec2f>(i, j)[0] = (u * P.at<cv::Vec2f>(i, j)[1] + v * Q.at<cv::Vec2f>(i, j)[1]) / d;
                    Z.at<cv::Vec2f>(i, j)[1] = (-u * P.at<cv::Vec2f>(i, j)[0] - v * Q.at<cv::Vec2f>(i, j)[0]) / d;
                }
            }
        }

        /* setting unknown average height to zero */
        Z.at<cv::Vec2f>(0, 0)[0] = 0.0f;
        Z.at<cv::Vec2f>(0, 0)[1] = 0.0f;

        cv::dft(Z, Z, cv::DFT_INVERSE | cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);

        return Z;
    }

    void checkSolverInfo(const Eigen::ComputationInfo& info) {
        switch (info) {
        case Eigen::Success:
            std::cout << "Solver info: Success" << std::endl;
            break;
        case Eigen::NumericalIssue:
            std::cout << "Solver info: Numerical Issue" << std::endl;
            break;
        case Eigen::NoConvergence:
            std::cout << "Solver info: No Convergence" << std::endl;
            break;
        case Eigen::InvalidInput:
            std::cout << "Solver info: Invalid Input" << std::endl;
            break;
        default:
            std::cout << "Solver info: Unknown issue" << std::endl;
            break;
        }
    }

    void printSparseMatrix(const Eigen::SparseMatrix<float>& mat) {
        Eigen::MatrixXf denseMat = Eigen::MatrixXf(mat);
        std::cout << "Matrix (" << mat.rows() << " x " << mat.cols() << "):" << std::endl;
        for (int i = 0; i < denseMat.rows(); ++i) {
            for (int j = 0; j < denseMat.cols(); ++j) {
                std::cout << denseMat(i, j) << "\t";
            }
            std::cout << std::endl;
        }
    }

    VectorXf PhotometricSteroCZX::globalHeightsMLS()
    {
        int M = Pgrads.rows * Pgrads.cols;
        cv::Mat P(Pgrads.rows * Pgrads.cols, 1, CV_32F, cv::Scalar::all(0));
        cv::Mat Q(Pgrads.rows * Pgrads.cols, 1, CV_32F, cv::Scalar::all(0));
        P = Pgrads.reshape(1, 1);
        Q = Qgrads.reshape(1, 1);


        Eigen::Map<Eigen::VectorXf> p_eigen(P.ptr<float>(), P.cols);

        Eigen::Map<Eigen::VectorXf> q_eigen(Q.ptr<float>(), Q.cols);
        Eigen::VectorXf b(p_eigen.size() + q_eigen.size());
        b << p_eigen, q_eigen;

        //// 创建稀疏矩阵的插入器
        //typedef Eigen::Triplet<float> T;
        //std::vector<T> tripletList;
        //tripletList.reserve(4 * M);
        Eigen::SparseMatrix<float> A(2 * M, M);

        //i, j+1 
        //#pragma omp parallel for
        for (int row = 0; row < Pgrads.rows; row++)
        {
            for (int column = 0; column < Pgrads.cols - 1; column++)
            {
                int index = column + row * Pgrads.cols;
                A.insert(index, index) = 1;
                A.insert(index, index + 1) = -1;
            }
            int index = (row + 1) * Pgrads.cols - 1;
            A.insert(index, index) = 1;
            A.insert(index, index - 2) = -1;
        }

        //i+1, j 
        //#pragma omp parallel for
        for (int row = 0; row < Pgrads.rows - 1; row++)
        {
            for (int column = 0; column < Pgrads.cols; column++)
            {
                int index = column + row * Pgrads.cols;
                A.insert(index + M, index) = 1;
                A.insert(index + M, index + Pgrads.cols) = -1;
            }
        }
        for (int column = 0; column < Pgrads.cols; column++)
        {
            int index = (Pgrads.rows - 1) * Pgrads.cols + column;
            A.insert(index + M, index) = 1;
            A.insert(index + M, index - 2 * Pgrads.cols) = -1;
        }
        //printSparseMatrix(A);
        Eigen::SparseLU<Eigen::SparseMatrix<float>> solver;
        solver.analyzePattern(A);
        solver.factorize(A);
        checkSolverInfo(solver.info());
        if (solver.info() != Eigen::Success) {
            throw std::runtime_error("Solving failed!");
        }
        Eigen::VectorXf Z = solver.solve(b);
        //A.at<float>(M-1 + M, M-1) = -1;
    //A.at<float>(M-1 + M, M-1 - Pgrads.cols) = 1;

        //cv::Mat B;
        //cv::hconcat(P, Q, B);

        //int shape[] = { 2 * M, M };
        ////cv::SparseMat A(2, shape, CV_32F);
        //cv::Mat A(2 * M, M, CV_32F, cv::Scalar::all(0));
        //
        ////边界换方向
        //A.at<float>(M-1, M-1) = -1.0f;
        //A.at<float>(M-1, M-2) = 1.0f;

        //for (int i = 0; i < M-1; i++)
        //{
        //    A.at<float>(i, i) = -1;
        //    A.at<float>(i, i+1) = 1;
        //}

        //A.at<float>(M-1 + M, M-1) = -1;
        //A.at<float>(M-1 + M, M-1 - Pgrads.cols) = 1;
        //for (int i = 0; i < M-1; i++)
        //{
        //    A.at<float>(i+M, i) = -1;
        //    A.at<float>(i+M, i+ Pgrads.cols) = 1;
        //}
        //
        //cv::solve(A, Z, B);
        //Z.reshape(1, Pgrads.rows);

        return Z;
    }

    CP PhotometricSteroCZX::getCloud(float scale)
    {
        //auto Z = globalHeightsMLS();
        auto Z = globalHeights();

        CP cloud(new CloudT);
        for (int y = 0; y < Normals.rows; ++y)
        {
            for (int x = 0; x < Normals.cols; ++x)
            {
                //float depth = Z[y * Normals.cols + x] * scale;
                float depth = Z.at<float>(y, x) * scale;

                //// 过滤掉无效深度值
                //if (depth <= 0.0f) {
                //    continue;
                //}

                // 将深度图像的像素点转换为3D点云
                pcl::PointXYZ point;
                point.x = static_cast<float>(x);
                point.y = static_cast<float>(y);
                point.z = depth;

                cloud->points.push_back(point);
            }
        }

        // 设置点云的宽和高
        cloud->width = static_cast<uint32_t>(cloud->points.size());
        cloud->height = 1;
        cloud->is_dense = false;

        return cloud;
    }

    cv::Mat PhotometricSteroCZX::getNormal()
    {
        cv::Mat Normalmap;
        cv::cvtColor(Normals, Normalmap, COLOR_BGR2RGB);
        //cout << Normals.at<float>(10, 10) << endl;
        //cv::cvtColor(Normals, Normalmap, COLOR_BGR2GRAY);
        return Normalmap;
    }

    cv::Mat PhotometricSteroCZX::getNormalGray()
    {
        cv::Mat Normalmap;
        cv::cvtColor(Normals, Normalmap, COLOR_BGR2GRAY);
        return Normalmap;
    }

    cv::Mat PhotometricSteroCZX::getMean()
    {
        vector<Mat> images;
        for (auto& path : img_paths)
        {
            auto img = cv::imread(path);
            images.push_back(img);
        }

        int height = images[0].rows;
        int width = images[0].cols;
        Mat mean = Mat(height, width, CV_32FC3, cv::Scalar::all(0));
        for (int i = 0; i < images.size(); i++)
        {
            auto img = images[i];
            cv::Mat temp;
            img.convertTo(temp, CV_32FC3);
            mean += temp;
        }
        mean = mean / static_cast<float>(images.size()) / 125;
        return mean;
    }

    cv::Mat PhotometricSteroCZX::getAlbedo()
    {
        return Albedo;
    }

    void PhotometricSteroCZX::MyShow(string name, Mat image) {
        if (image.cols > 2048) {
            resize(image, image, Size(image.cols / 4, image.rows / 4));
        }
        imshow(name, image);
        waitKey(0);
        return;
    }
}