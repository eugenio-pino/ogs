/**
 * \copyright
 * Copyright (c) 2012-2015, OpenGeoSys Community (http://www.opengeosys.org)
 *			Distributed under a Modified BSD License.
 *			  See accompanying file LICENSE.txt or
 *			  http://www.opengeosys.org/LICENSE.txt
 */

#include <ctime>
#include <numeric>
#include "gtest/gtest.h"

#include "MeshLib/MeshGenerators/MeshGenerator.h"
#include "MeshLib/Mesh.h"
#include "MeshLib/Elements/Line.h"
#include "Location.h"
#include "PropertyVector.h"

#ifdef OGS_USE_EIGEN
#include <Eigen/Eigen>
#endif

class MeshLibProperties : public ::testing::Test
{
public:
	MeshLibProperties()
		: mesh(nullptr)
	{
		mesh = MeshLib::MeshGenerator::generateRegularHexMesh(1.0, mesh_size);
	}

	~MeshLibProperties()
	{
		delete mesh;
	}

	static std::size_t const mesh_size = 5;
	MeshLib::Mesh * mesh;
};
std::size_t const MeshLibProperties::mesh_size;

TEST_F(MeshLibProperties, PropertyVectorTestMetaData)
{
	ASSERT_TRUE(mesh != nullptr);

	std::string const prop_name("TestProperty");
	boost::optional<MeshLib::PropertyVector<double> &> p(
		mesh->getProperties().createNewPropertyVector<double>(prop_name,
			MeshLib::MeshItemType::Cell)
	);

	ASSERT_EQ(0u, (*p).getPropertyName().compare(prop_name));
	ASSERT_EQ(MeshLib::MeshItemType::Cell, (*p).getMeshItemType());
	ASSERT_EQ(1u, (*p).getTupleSize());
	ASSERT_EQ(0u, (*p).size());
}

TEST_F(MeshLibProperties, AddDoubleProperties)
{
	ASSERT_TRUE(mesh != nullptr);
	const std::size_t size(mesh_size*mesh_size*mesh_size);

	std::string const prop_name("TestProperty");
	boost::optional<MeshLib::PropertyVector<double> &> double_properties(
		mesh->getProperties().createNewPropertyVector<double>(prop_name,
			MeshLib::MeshItemType::Cell)
	);
	(*double_properties).resize(size);
	std::iota((*double_properties).begin(), (*double_properties).end(), 1);

	boost::optional<MeshLib::PropertyVector<double> const&>
		double_properties_cpy(mesh->getProperties().getPropertyVector<double>(
			prop_name));
	ASSERT_FALSE(!double_properties_cpy);

	for (std::size_t k(0); k<size; k++) {
		ASSERT_EQ((*double_properties)[k], (*double_properties_cpy)[k]);
	}

	mesh->getProperties().removePropertyVector(prop_name);
	boost::optional<MeshLib::PropertyVector<double> const&>
		removed_double_properties(
			mesh->getProperties().getPropertyVector<double>(prop_name)
		);

	ASSERT_TRUE(!removed_double_properties);
}

TEST_F(MeshLibProperties, AddDoublePointerProperties)
{
	ASSERT_TRUE(mesh != nullptr);
	std::string const& prop_name("GroupProperty");
	// check if a property with the name is already assigned to the mesh
	ASSERT_FALSE(mesh->getProperties().hasPropertyVector(prop_name));
	// data needed for the property
	const std::size_t n_prop_val_groups(10);
	const std::size_t n_items(mesh_size*mesh_size*mesh_size);
	std::vector<std::size_t> prop_item2group_mapping(n_items);
	// create simple mat_group to index mapping
	for (std::size_t j(0); j<n_prop_val_groups; j++) {
		std::size_t const lower(
			static_cast<std::size_t>(
				(static_cast<double>(j)/n_prop_val_groups)*n_items
			)
		);
		std::size_t const upper(
			static_cast<std::size_t>(
				(static_cast<double>(j+1)/n_prop_val_groups)*n_items
			)
		);
		for (std::size_t k(lower); k<upper; k++) {
			prop_item2group_mapping[k] = j;
		}
	}
	// obtain PropertyVector data structure
	boost::optional<MeshLib::PropertyVector<double*> &> group_properties(
		mesh->getProperties().createNewPropertyVector<double*>(
			prop_name, n_prop_val_groups, prop_item2group_mapping,
			MeshLib::MeshItemType::Cell
		)
	);
	// initialize the property values
	for (std::size_t i(0); i<n_prop_val_groups; i++) {
		(*group_properties).initPropertyValue(i, i+1);
	}

	// the mesh should have the property assigned to cells
	ASSERT_TRUE(mesh->getProperties().hasPropertyVector(prop_name));
	// fetch the properties from the container
	boost::optional<MeshLib::PropertyVector<double*> const&>
		group_properties_cpy(mesh->getProperties().getPropertyVector<double*>(
			prop_name));
	ASSERT_FALSE(!group_properties_cpy);

	for (std::size_t k(0); k<n_items; k++) {
		ASSERT_EQ((*group_properties)[k], (*group_properties_cpy)[k]);
	}

	mesh->getProperties().removePropertyVector(prop_name);
	boost::optional<MeshLib::PropertyVector<double*> const&>
		removed_group_properties(mesh->getProperties().getPropertyVector<double*>(
			prop_name));

	ASSERT_TRUE(!removed_group_properties);
}

TEST_F(MeshLibProperties, AddArrayPointerProperties)
{
	ASSERT_TRUE(mesh != nullptr);
	std::string const& prop_name("GroupPropertyWithArray");
	const std::size_t n_prop_val_groups(10);
	const std::size_t n_items(mesh_size*mesh_size*mesh_size);
	std::vector<std::size_t> prop_item2group_mapping(n_items);
	// create simple mat_group to index mapping
	for (std::size_t j(0); j<n_prop_val_groups; j++) {
		std::size_t const lower(
			static_cast<std::size_t>(
				(static_cast<double>(j)/n_prop_val_groups)*n_items
			)
		);
		std::size_t const upper(
			static_cast<std::size_t>(
				(static_cast<double>(j+1)/n_prop_val_groups)*n_items
			)
		);
		for (std::size_t k(lower); k<upper; k++) {
			prop_item2group_mapping[k] = j;
		}
	}
	boost::optional<MeshLib::PropertyVector<std::array<double,3>*> &>
		group_prop_vec(
			mesh->getProperties().createNewPropertyVector<std::array<double,3>*>(
				prop_name, n_prop_val_groups, prop_item2group_mapping,
				MeshLib::MeshItemType::Cell
			)
		);
	// initialize the property values
	for (std::size_t i(0); i<n_prop_val_groups; i++) {
		(*group_prop_vec).initPropertyValue(i,
			std::array<double,3>({{static_cast<double>(i),
				static_cast<double>(i+1),
				static_cast<double>(i+2)}}
			)
		);
	}

	boost::optional<MeshLib::PropertyVector<std::array<double,3>*> const&>
		group_properties_cpy(
			mesh->getProperties().getPropertyVector<std::array<double,3>*>(prop_name)
		);
	ASSERT_FALSE(!group_properties_cpy);

	for (std::size_t k(0); k<n_items; k++) {
		ASSERT_EQ((*((*group_prop_vec)[k]))[0],
			(*((*group_properties_cpy)[k]))[0]);
		ASSERT_EQ((*((*group_prop_vec)[k]))[1],
			(*((*group_properties_cpy)[k]))[1]);
		ASSERT_EQ((*((*group_prop_vec)[k]))[2],
			(*((*group_properties_cpy)[k]))[2]);
	}

	mesh->getProperties().removePropertyVector(prop_name);
	boost::optional<MeshLib::PropertyVector<std::array<double, 3>*> const&>
		removed_group_properties(
			mesh->getProperties().getPropertyVector<std::array<double,3>*>(prop_name)
		);

	ASSERT_TRUE(!removed_group_properties);
}

TEST_F(MeshLibProperties, AddVariousDifferentProperties)
{
	ASSERT_TRUE(mesh != nullptr);

	std::string const& prop_name("GroupVectorProperty");
	// check if the property is already assigned to the mesh
	ASSERT_FALSE(mesh->getProperties().hasPropertyVector(prop_name));
	const std::size_t n_prop_val_groups(10);
	const std::size_t n_items(mesh_size*mesh_size*mesh_size);
	std::vector<std::size_t> prop_item2group_mapping(n_items);
	// create simple mat_group to index mapping
	for (std::size_t j(0); j<n_prop_val_groups; j++) {
		std::size_t const lower(
			static_cast<std::size_t>(
				(static_cast<double>(j)/n_prop_val_groups)*n_items
			)
		);
		std::size_t const upper(
			static_cast<std::size_t>(
				(static_cast<double>(j+1)/n_prop_val_groups)*n_items
			)
		);
		for (std::size_t k(lower); k<upper; k++) {
			prop_item2group_mapping[k] = j;
		}
	}
	// create data structure for the property
	boost::optional<MeshLib::PropertyVector<std::array<double,3>*> &>
		group_properties(
			mesh->getProperties().createNewPropertyVector<std::array<double,3>*>(
				prop_name, n_prop_val_groups, prop_item2group_mapping,
				MeshLib::MeshItemType::Cell
			)
		);
	// initialize the property values
	for (std::size_t i(0); i<n_prop_val_groups; i++) {
		(*group_properties).initPropertyValue(i,
			std::array<double,3>({{static_cast<double>(i),
				static_cast<double>(i+1),
				static_cast<double>(i+2)}}
			)
		);
	}

	// the mesh should have the property assigned to cells
	ASSERT_TRUE(mesh->getProperties().hasPropertyVector(prop_name));

	// fetch the vector filled with property values from mesh
	boost::optional<MeshLib::PropertyVector<std::array<double,3>*> const&>
		group_properties_cpy(
			mesh->getProperties().getPropertyVector<std::array<double,3>*>(prop_name)
		);
	ASSERT_FALSE(!group_properties_cpy);
	// compare the content
	const std::size_t n_elements(mesh_size*mesh_size*mesh_size);
	for (std::size_t k(0); k<n_elements; k++) {
		ASSERT_EQ((*((*group_properties)[k]))[0],
			(*((*group_properties_cpy)[k]))[0]);
		ASSERT_EQ((*((*group_properties)[k]))[1],
			(*((*group_properties_cpy))[k])[1]);
		ASSERT_EQ((*((*group_properties)[k]))[2],
			(*((*group_properties_cpy)[k]))[2]);
	}

	// *** add a 2nd property ***
	std::string const& prop_name_2("ItemwiseMatrixProperties");
	// check if the property is already assigned to the mesh
	ASSERT_FALSE(mesh->getProperties().hasPropertyVector(prop_name_2));
	const std::size_t n_items_2(mesh_size*mesh_size*mesh_size);
	boost::optional<MeshLib::PropertyVector<std::array<float,9>> &>
		array_properties(mesh->getProperties().createNewPropertyVector<
			std::array<float,9>
		> (prop_name_2, MeshLib::MeshItemType::Cell)
	);

	(*array_properties).resize(n_items_2);

	// initialize the property values
	for (std::size_t i(0); i<n_items_2; i++) {
		// init property value
		for (std::size_t k(0); k<(*array_properties)[i].size(); k++) {
			(*array_properties)[i][k] = static_cast<float>(i+k);
		}
	}

	EXPECT_EQ(9, (*array_properties)[0].size());

	// the mesh should have the property assigned to cells
	ASSERT_TRUE(mesh->getProperties().hasPropertyVector(prop_name_2));

	// fetch the vector in order to compare the content
	boost::optional<MeshLib::PropertyVector<std::array<float,9>> const&>
		array_properties_cpy(mesh->getProperties().getPropertyVector<std::array<float,9>>(
			prop_name_2)
	);
	ASSERT_FALSE(!array_properties_cpy);

	// compare the values/matrices
	for (std::size_t k(0); k<n_items_2; k++) {
		for (std::size_t j(0); j<(*array_properties)[k].size(); j++) {
			ASSERT_EQ((*array_properties)[k][j], (*array_properties_cpy)[k][j]);
		}
	}

	// *** add a 3rd property ***
#ifdef OGS_USE_EIGEN
	std::string const& prop_name_3("ItemwiseEigenMatrixProperties");
	// check if the property is already assigned to the mesh
	ASSERT_FALSE(mesh->getProperties().hasPropertyVector(prop_name_3));
	boost::optional<
		MeshLib::PropertyVector<Eigen::Matrix<double,3,3,Eigen::RowMajor>> &>
	matrix_properties(mesh->getProperties().createNewPropertyVector
		<Eigen::Matrix<double,3,3,Eigen::RowMajor>> (
			prop_name_3, MeshLib::MeshItemType::Cell)
	);
	// init property values
	for (auto it=matrix_properties->begin(); it != matrix_properties->end(); it++)
	{
		for (int r(0); r<it->rows(); r++) {
			for (int c(0); c<it->cols(); c++) {
				(*it)(r,c) = static_cast<double>(
					std::distance(matrix_properties->begin(),it)+r*it->cols()+c+1);
			}
		}
	}

	// the mesh should have the property assigned to cells
	ASSERT_TRUE(mesh->getProperties().hasPropertyVector(prop_name_3));

	// fetch the vector in order to compare the content
	boost::optional<
		MeshLib::PropertyVector<Eigen::Matrix<double,3,3,Eigen::RowMajor>> const&
	> matrix_properties_cpy(
		mesh->getProperties().getPropertyVector<Eigen::Matrix<double,3,3,Eigen::RowMajor>>(
			prop_name_3)
	);
	ASSERT_FALSE(!matrix_properties_cpy);

	// compare the values/matrices
	auto it_cpy=matrix_properties_cpy->begin();
	for (auto it=matrix_properties->begin(); it != matrix_properties->end();
		it++, it_cpy++) {
		for (int r(0); r<it->rows(); r++) {
			for (int c(0); c<it->cols(); c++) {
				ASSERT_EQ((*it)(r,c), (*it_cpy)(r,c));
			}
		}
	}
#endif
}

TEST_F(MeshLibProperties, CopyConstructor)
{
	ASSERT_TRUE(mesh != nullptr);
	std::string const& prop_name("GroupProperty");
	// data needed for the property
	const std::size_t n_prop_val_groups(10);
	const std::size_t n_items(mesh_size*mesh_size*mesh_size);
	std::vector<std::size_t> prop_item2group_mapping(n_items);
	// create simple mat_group to index mapping
	for (std::size_t j(0); j<n_prop_val_groups; j++) {
		std::size_t const lower(
			static_cast<std::size_t>(
				(static_cast<double>(j)/n_prop_val_groups)*n_items
			)
		);
		std::size_t const upper(
			static_cast<std::size_t>(
				(static_cast<double>(j+1)/n_prop_val_groups)*n_items
			)
		);
		for (std::size_t k(lower); k<upper; k++) {
			prop_item2group_mapping[k] = j;
		}
	}
	// obtain PropertyVector data structure
	boost::optional<MeshLib::PropertyVector<double*> &> group_properties(
		mesh->getProperties().createNewPropertyVector<double*>(
			prop_name, n_prop_val_groups, prop_item2group_mapping,
			MeshLib::MeshItemType::Cell
		)
	);
	// initialize the property values
	for (std::size_t i(0); i<n_prop_val_groups; i++) {
		(*group_properties).initPropertyValue(i, i+1);
	}

	// create a copy from the original Properties object
	MeshLib::Properties properties_copy(mesh->getProperties());
	// check if the Properties have a PropertyVector with the correct name
	ASSERT_TRUE(properties_copy.hasPropertyVector(prop_name));
	// fetch the PropertyVector from the copy of the Properties object
	boost::optional<MeshLib::PropertyVector<double*> const&>
		group_properties_cpy(properties_copy.getPropertyVector<double*>(
			prop_name));
	ASSERT_FALSE(!group_properties_cpy);

	// check if the values in the PropertyVector of the copy of the Properties
	// are the same
	for (std::size_t k(0); k<n_items; k++) {
		EXPECT_EQ(*(*group_properties)[k], *(*group_properties_cpy)[k]);
	}
}

