//https://blog.csdn.net/u012541187/article/details/53259821
#include <pcl/point_types.h>
#include <pcl/io/PLY_io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/features/normal_3d_omp.h>
#include <pcl/features/normal_3d.h>
#include <pcl/surface/gp3.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <boost/thread/thread.hpp>
#include <fstream>

void main()
{
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	if (pcl::io::loadPLYFile("bunny.ply", *cloud) == -1)
	{
		PCL_ERROR("Could not read pcd file!\n");
		return;
	}
	pcl::PointCloud<pcl::PointNormal>::Ptr cloud_with_normals(new pcl::PointCloud<pcl::PointNormal>);
	pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> n;//���߹��ƶ���
	pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);//�洢���Ƶķ���
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
	tree->setInputCloud(cloud);
	n.setInputCloud(cloud);
	n.setSearchMethod(tree);
	n.setKSearch(20);
	n.compute(*normals);
	pcl::concatenateFields(*cloud, *normals, *cloud_with_normals);
	pcl::search::KdTree<pcl::PointNormal>::Ptr tree2(new pcl::search::KdTree<pcl::PointNormal>);
	tree2->setInputCloud(cloud_with_normals);
	pcl::GreedyProjectionTriangulation<pcl::PointNormal> gp3;
	pcl::PolygonMesh mesh; //�洢�������ǻ�������ģ��
	gp3.setSearchRadius(2);//���������Ҫ����
	gp3.setMu(2.5);//�����������������ڽ������Զ����Ϊ2.5 
	gp3.setMaximumNearestNeighbors(100);//�����������������������Ϊ100
	gp3.setMaximumSurfaceAngle(M_PI / 3);//����ĳ�㷨�߷���ƫ�������㷨�߷�������Ƕ�Ϊ45��
	gp3.setMinimumAngle(M_PI / 180);//�������ǻ���õ����������ڽ���С�Ƕ�Ϊ10��
	gp3.setMaximumAngle(2 * M_PI / 3);
	gp3.setNormalConsistency(false);//���øò�����֤���߳���һ��
	gp3.setInputCloud(cloud_with_normals);//�����������Ϊ�������
	gp3.setSearchMethod(tree2);//�������ط�ʽΪtree2
	gp3.reconstruct(mesh);//�ؽ���ȡ���ǻ�

	//�������񻯽��
	pcl::io::savePLYFile("mesh_result2.ply", mesh);

	std::vector<int> parts = gp3.getPartIDs();

	std::vector<int> status = gp3.getPointStates();
	fstream fs;
	fs.open("partsID.txt", ios::out);

	if (!fs)
	{
		return;
	}

	fs << "��������Ϊ��" << parts.size() << "\n";

	for (int i = 0; i < parts.size(); i++)
	{
		if (parts[i] != 0)
		{
			fs << parts[i] << "\n";

		}
	}

	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3Dviewer"));
	viewer->setBackgroundColor(0, 0, 0);
	viewer->addPolygonMesh(mesh, "W");
	viewer->initCameraParameters();
	while (!viewer->wasStopped())
	{
		viewer->spinOnce(100);
		boost::this_thread::sleep(boost::posix_time::microseconds(50000));
	}
	return;
}

