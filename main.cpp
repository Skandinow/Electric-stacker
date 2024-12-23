#include <iostream>
#include <fstream>
#include <string>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/linestring.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace bg = boost::geometry;

struct MyPoint {
public:
    MyPoint() = default;
    MyPoint(double initx, double inity) : x(initx), y(inity) {}

    double x = 0.0;
    double y = 0.0;

    friend bool operator==(const MyPoint& lhs, const MyPoint& rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
};

typedef std::vector<MyPoint> MyPolygon;

BOOST_GEOMETRY_REGISTER_POINT_2D(MyPoint, double, bg::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_LINESTRING(MyPolygon)

std::vector<MyPolygon> load_polygons_from_json(const std::string& filename) {
    std::ifstream f(filename);
    json data = json::parse(f);

    std::vector<MyPolygon> polygons;

    int i = 0;
    for (auto& feature : data["features"]) {
        auto& coordinates = feature["geometry"]["coordinates"];
        i++;
        MyPolygon poly;
        for (auto& coord : coordinates[0]) {
            double x = coord[0];
            double y = coord[1];

            poly.emplace_back(x, y);
        }
        polygons.push_back(std::move(poly));
    }

    return polygons;
}


void print_polygons(std::vector<MyPolygon> &polygons) {
    int j = 0;
    std::cout << "------BEGIN POLYGON PRINT------\n";

    for (const auto &polygon: polygons) {
        std::cout << j << "th polygon\n";
        for (const auto &point: polygon) {
            std::cout << "(" << point.x << ", " << point.y << ")\n";
        }
        std::cout << "\n\n";

        j++;
    }
    std::cout << "------END POLYGON PRINT------\n\n";

}

void intersect_all_polygons(const std::vector<MyPolygon>& polygons,
                            std::vector<std::deque<MyPolygon>>& results) {
    for (size_t i = 0; i < polygons.size(); ++i) {
        for (size_t j = i + 1; j < polygons.size(); ++j) {
            std::deque<MyPolygon> output;
            bg::intersection(polygons[i], polygons[j], output);
            if (!output.empty()) {
                results.push_back(output);
            }
        }
    }
}

void print_intersection(const std::vector<std::deque<MyPolygon>> &intersection_results) {
    for (size_t k = 0; k < intersection_results.size(); ++k) {
        std::cout << "Result " << k << ": \n";
        for (const auto& polygon : intersection_results[k]) {
            for (const auto& point : polygon) {
                std::cout << "(" << point.x << ", " << point.y << ") ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

bool is_point_in_any_intersection_result(const MyPoint& point, const std::vector<std::deque<MyPolygon>>& intersection_results) {
    for (const auto& result_deque : intersection_results) {
        for (const auto& polygon : result_deque) {
            if (bg::within(point, polygon)) {
                return true;
            }
        }
    }
    return false;
}

void print_new_verticles(const std::vector<MyPolygon> &new_polygons) {// Выводим новые полигоны
    std::cout << "New polys:\n";
    for (size_t i = 0; i < new_polygons.size(); ++i) {
        std::cout << "New poly " << i << ":\n";
        for (const auto& point : new_polygons[i]) {
            std::cout << "(" << point.x << ", " << point.y << ")\n";
        }
        std::cout << "\n";
    }
}

std::vector<MyPolygon>
get_new_verticals(std::vector<MyPolygon> &polygons, const std::vector<std::deque<MyPolygon>> &intersection_results) {
    std::vector<MyPolygon> new_polygons;

    for (auto &polygon: polygons) {
        MyPolygon new_poly;
        for (auto &point: polygon) {
            if (!is_point_in_any_intersection_result(point, intersection_results)) {
                new_poly.push_back(point);
            }
        }
        new_polygons.push_back(new_poly);
    }
    return new_polygons;
}

std::string polygonToString(const MyPolygon& polygon) {
    std::ostringstream ss;
    ss << "[";
    for (size_t i = 0; i < polygon.size(); ++i) {
        ss << "[" << polygon[i].x << ", " << polygon[i].y << "]";
        if (i != polygon.size() - 1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}

void write_to_json(std::vector<MyPolygon> &new_polygons) {
    json result;
    result["type"] = "FeatureCollection";
    result["name"] = "roads1";
    result["crs"] = R"({"type": "name", "properties": { "name": "urn:ogc:def:crs:EPSG::3857" }})";


    for (auto& polygon : new_polygons) {
        json feature;
        feature["type"] = "Feature";
        feature["properties"] = {};
        feature["geometry"]["type"] = "Polygon";

        feature["geometry"]["coordinates"] = polygonToString(polygon);
        result["features"].push_back(feature);
    }

    // Открываем или создаем файл result.json и записываем в него данные
    std::ofstream out("result.json");
    out << result.dump();
}

int main() {
    std::string line;
    std::ifstream in("config.txt"); // открываем файл для чтения
    if (in.is_open())
    {
        double trenchValue, HDDValue, transitValue, minHDDLength, maxHDDLength, degreeDiviation;
        in >> trenchValue >> HDDValue >> transitValue;
        in >> minHDDLength >> maxHDDLength;
        in >> degreeDiviation;

        std::cout << trenchValue << " " << HDDValue << " " << transitValue << " "
        << minHDDLength << " " << maxHDDLength << " " << degreeDiviation << " " << std::endl;

    }
    in.close();     // закрываем файл

    std::vector<MyPolygon> polygons = load_polygons_from_json("test.json");
    print_polygons(polygons);

    std::vector<std::deque<MyPolygon>> intersection_results;
    // Выполним пересечение каждого полигона с каждым другим
    intersect_all_polygons(polygons, intersection_results);

    print_intersection(intersection_results);

    // Найдем новые вершины
    std::vector<MyPolygon> new_polygons = get_new_verticals(polygons, intersection_results);
    print_new_verticles(new_polygons);

    write_to_json(new_polygons);
}
