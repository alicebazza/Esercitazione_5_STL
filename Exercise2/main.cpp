#include <iostream>
#include <list>
#include <map>
#include "PolygonalMesh.hpp"
#include "Utils.hpp"
#include "UCDUtilities.hpp"

using namespace std;
using namespace Eigen;
using namespace PolygonalLibrary;

bool NonZeroLength(const PolygonalMesh& mesh)
{
	const double epsilon = 1e-10;
    for (unsigned int i = 0; i < mesh.NumCell1Ds; ++i) {
        unsigned int originId = mesh.Cell1DsExtrema(0, i);
        unsigned int endId = mesh.Cell1DsExtrema(1, i);

        Vector3d origin = mesh.Cell0DsCoordinates.col(originId);
        Vector3d end = mesh.Cell0DsCoordinates.col(endId);

        double length = (end - origin).norm();
        if (length < epsilon) {
            cerr << "Errore: il segmento " << i << " ha lunghezza nulla" << endl;
            return false;
        }
    }
    return true; // Se tutti i segmenti hanno lunghezza non nulla
}

bool NonZeroArea(const PolygonalMesh& mesh)
{
	const double epsilon = 1e-10;
	for (unsigned int i = 0; i < mesh.NumCell2Ds; ++i){
        const auto& vertices = mesh.Cell2DsVertices[i];
        if (vertices.size() < 3) continue;  // I poligoni con meno di 3 vertici non hanno area

        double area = 0.0;
        for (unsigned int j = 0; j < vertices.size(); ++j){
            unsigned int k = (j + 1) % vertices.size();
            const auto& v1 = mesh.Cell0DsCoordinates.col(vertices[j]);
            const auto& v2 = mesh.Cell0DsCoordinates.col(vertices[k]);

            area += v1.x() * v2.y() - v1.y() * v2.x();
        }
        area = std::abs(area) / 2.0;

        if (area < epsilon) {
            cerr << "Errore: poligono con indice " << i << " ha area nulla" << endl;
            return false;
        }
    }
    return true; // Se tutti i poligoni hanno area non nulla
}


int main()
{
    PolygonalMesh mesh;

    if(!ImportMesh(mesh))
    {
        cerr << "file not found" << endl;
        return 1;
    }

    Gedim::UCDUtilities utilities;
    {
        vector<Gedim::UCDProperty<double>> cell0Ds_properties(1);

        cell0Ds_properties[0].Label = "Marker";
        cell0Ds_properties[0].UnitLabel = "-";
        cell0Ds_properties[0].NumComponents = 1;

        vector<double> cell0Ds_marker(mesh.NumCell0Ds, 0.0);
        for(const auto &m : mesh.MarkerCell0Ds)
            for(const unsigned int id: m.second)
                cell0Ds_marker.at(id) = m.first;

        cell0Ds_properties[0].Data = cell0Ds_marker.data();

        utilities.ExportPoints("./Cell0Ds.inp",
                               mesh.Cell0DsCoordinates,
                               cell0Ds_properties);
    }

    {
        vector<Gedim::UCDProperty<double>> cell1Ds_properties(1);

        cell1Ds_properties[0].Label = "Marker";
        cell1Ds_properties[0].UnitLabel = "-";
        cell1Ds_properties[0].NumComponents = 1;

        vector<double> cell1Ds_marker(mesh.NumCell1Ds, 0.0);
        for(const auto &m : mesh.MarkerCell1Ds)
            for(const unsigned int id: m.second)
                cell1Ds_marker.at(id) = m.first;

        cell1Ds_properties[0].Data = cell1Ds_marker.data();

        utilities.ExportSegments("./Cell1Ds.inp",
                                 mesh.Cell0DsCoordinates,
                                 mesh.Cell1DsExtrema,
                                 {},
                                 cell1Ds_properties);
     }
     
    // controllo sui marker
    map<unsigned int, list<unsigned int>> MarkerCell0Ds_true = {
    {1, {0}},
    {2, {1}},
    {3, {2}},
    {4, {3}},
    {5, {6, 16, 24}},
    {6, {7, 17, 22, 78}},
    {7, {8, 20, 23, 52, 59}},
    {8, {5, 15, 21, 26, 92}}
    };
    
    if ( mesh.MarkerCell0Ds == MarkerCell0Ds_true) {
        cout << "Marker celle 0D corretti" << endl;
    } else {
        cout << "Marker celle 0D errati" << endl;
    }
    
    map<unsigned int, list<unsigned int>> MarkerCell1Ds_true = {
	{5, {8,19,22,28}},
    {6, {6, 23, 26,126,127}},
    {7, {14,17,24,79,92, 93}},
    {8, {11,25,29,30,159,160}},
    };
    
    if ( mesh.MarkerCell1Ds == MarkerCell1Ds_true) {
        cout << "Marker celle 1D corretti" << endl;
    } else {
        cout << "Marker celle 1D errati" << endl;
    }
    
    map<unsigned int, list<unsigned int>> MarkerCell2Ds_true = {};
    
    if ( mesh.MarkerCell2Ds == MarkerCell2Ds_true) {
        cout << "Marker celle 2D corretti" << endl;
    } else {
        cout << "Marker celle 2D errati" << endl;
    }
    
    // controllo sui lati
    bool result1 = NonZeroLength(mesh);
    if (result1) {
        cout << "Tutti i segmenti hanno lunghezza non nulla" << endl;
    }
    
    // controllo sulle aree
    bool result2 = NonZeroArea(mesh);
    if (result2) {
        cout << "Tutti i poligoni hanno area non nulla" << endl;
    }

    return 0;
}
