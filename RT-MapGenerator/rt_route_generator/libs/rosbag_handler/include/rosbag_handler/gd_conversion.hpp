#include <cmath>

const double EARTH_RADIUS_M = 6378137.0;

inline double deg2rad(double deg) {
    return deg * M_PI / 180.0;
}

inline double rad2deg(double rad) {
    return rad * 180.0 / M_PI;
}

// Calculates the Haversine distance between two points in meters
inline double haversine_distance(double lat1, double lon1, double lat2, double lon2) {
    lat1 = deg2rad(lat1);
    lon1 = deg2rad(lon1);
    lat2 = deg2rad(lat2);
    lon2 = deg2rad(lon2);
     
    double dlong = lon2 - lon1;
    double dlat = lat2 - lat1;
    double a = pow(sin(dlat / 2), 2) +
                          cos(lat1) * cos(lat2) *
                          pow(sin(dlong / 2), 2);
    double c = 2 * asin(sqrt(a));
    double distance = c * EARTH_RADIUS_M;

    return distance;
}
