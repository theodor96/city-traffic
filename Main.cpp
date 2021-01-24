#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
    using City = std::uint64_t;
    using Neighbourhood = std::list<City>;
    using CityMap = std::unordered_map<City, Neighbourhood>;

    using Traffic = std::uint64_t;
    using TrafficMap = std::unordered_map<City, Traffic>;
    
    using CityPath = std::pair<City, City>;
    using CityPathTrafficCache = std::unordered_map<CityPath, Traffic>;
    
    using CityTraffic = std::pair<City, Traffic>;
    using TrafficResult = std::vector<CityTraffic>;
    
    using Input = std::vector<std::string>;
    using Output = std::string;
    using TestCase = std::pair<Input, Output>;
    using TestData = std::vector<TestCase>;
}

namespace std
{
    template <>
    struct hash<CityPath>
    {
        auto operator()(const CityPath& cityPath) const
        {
            const std::hash<City> hasher{};
            const auto firstHash = hasher(cityPath.first);
            
            return firstHash ^ (hasher(cityPath.second) + 0x9e3779b9 + (firstHash << 6) + (firstHash >> 2));
        }
    };
}

namespace
{
    CityMap cityMap{};
    TrafficMap trafficMap{};
    CityPathTrafficCache cityPathTrafficCache{};
}

void parseInput(const Input& input)
{
    for (const auto& description : input)
    {
        std::istringstream descriptionStream{description};
        City city{};
        descriptionStream >> city;

        auto& neighbourList = cityMap.emplace(city, Neighbourhood{}).first->second;
        if (description.find("[]") != std::string::npos)
        {
            continue;
        }

        char dummy{};
        descriptionStream >> dummy; // :
        descriptionStream >> dummy; // [

        do
        {
            descriptionStream >> city;
            neighbourList.emplace_back(city);

            descriptionStream >> dummy; // `,` or `]`
        }
        while (dummy == ',');
    }
}

Traffic computeTrafficFromNeighbourhood(City city, const Neighbourhood& neighbourhood)
{
    Traffic traffic{};
    for (const auto neighbour : neighbourhood)
    {
        if (neighbour != city)
        {
            traffic += neighbour + trafficMap[neighbour];
        }
    }

    return traffic;
}

void traverseCitiesViaPath(const CityPath& cityPath)
{
    Traffic trafficMetWhileTraversing{};
    
    const auto trafficCacheItr = cityPathTrafficCache.find(cityPath);
    if (trafficCacheItr != cityPathTrafficCache.cend())
    {
        trafficMetWhileTraversing = trafficCacheItr->second;
    }
    else
    {
        const auto& neighbourhood = cityMap[cityPath.first];
        for (const auto neighbour : neighbourhood)
        {
            if (trafficMap.find(neighbour) == trafficMap.cend())
            {
                trafficMap.emplace(neighbour, Traffic{});
                traverseCitiesViaPath(std::make_pair(neighbour, cityPath.first));
            }
        }

        trafficMetWhileTraversing = computeTrafficFromNeighbourhood(cityPath.second, neighbourhood);
        cityPathTrafficCache.emplace(cityPath, trafficMetWhileTraversing);
    }
    
    trafficMap[cityPath.first] = trafficMetWhileTraversing;
}

Traffic computeMaximumTraffic(City city)
{
    traverseCitiesViaPath(std::make_pair(city, City{}));

    Traffic maxTraffic{};
    for (const auto neighbour : cityMap[city])
    {
        const auto traffic = trafficMap[neighbour] + neighbour;
        if (maxTraffic < traffic)
        {
            maxTraffic = traffic;
        }
    }

    return maxTraffic;
}

TrafficResult computeOverallTrafficResult()
{
    TrafficResult trafficResult{};
    trafficResult.reserve(cityMap.size());

    for (const auto& [city, _] : cityMap)
    {
        trafficResult.emplace_back(city, computeMaximumTraffic(city));
        trafficMap.clear();
    }

    std::sort(trafficResult.begin(),
              trafficResult.end(),
              [](const auto& lhs, const auto& rhs)
              {
                  return lhs.first < rhs.first;
              });
    
    return trafficResult;
}

std::string serializeTrafficResult(const TrafficResult& trafficResult)
{
    std::string serializedTrafficResult{};
    for (const auto& [city, traffic] : trafficResult)
    {
        serializedTrafficResult += std::to_string(city) + ":" + std::to_string(traffic) + ",";
    }

    if (!serializedTrafficResult.empty())
    {
        serializedTrafficResult.pop_back();
    }

    return serializedTrafficResult;
}

TestData getTestData()
{
    return TestData
    {
        std::make_pair(Input{"1:[2,7,8]", "2:[1,3,6]", "3:[2,4,5]", "4:[3]", "5:[3]", "6:[2]",
                             "7:[1]", "8:[1,9,12]", "9:[8,10,11]", "10:[9]", "11:[9]", "12:[8]", "13:[]"},
                       Output{"1:50,2:58,3:66,4:74,5:73,6:72,7:71,8:30,9:48,10:68,11:67,12:66,13:0"}),

        std::make_pair(Input{"1:[5]", "4:[5]", "3:[5]", "5:[1,4,3,2]",
                             "2:[5,15,7]", "7:[2,8]", "8:[7,38]", "15:[2]", "38:[8]"},
                       Output{"1:82,2:53,3:80,4:79,5:70,7:46,8:38,15:68,38:45"}),

        std::make_pair(Input{"1:[5]", "2:[5]", "3:[5]", "4:[5]", "5:[1,2,3,4]"},
                       Output{"1:14,2:13,3:12,4:11,5:4"}),

        std::make_pair(Input{"1:[5]", "2:[5,18]", "3:[5,12]", "4:[5]", "5:[1,2,3,4]", "18:[2]", "12:[3]"},
                       Output{"1:44,2:25,3:30,4:41,5:20,12:33,18:27"})
    };
}

void resetDataStructures()
{
    cityMap.clear();
    trafficMap.clear();
    cityPathTrafficCache.clear();
}

void runTestCases(const TestData& testData)
{
    std::cout << "\n\n";

    std::size_t testCaseIndex{};
    for (const auto& testCase : testData)
    {
        parseInput(testCase.first);
        std::cout << "test case #" << ++testCaseIndex << " ---> ";

        const auto trafficResult = computeOverallTrafficResult();
        const auto output = serializeTrafficResult(trafficResult);

        if (output == testCase.second)
        {
            std::cout << "CORRECT";
        }
        else
        {
            std::cout << "WRONG (got " << output << " but expected " << testCase.second;
        }

        std::cout << "\n\n";
        resetDataStructures();
    }

    std::cout << std::flush;
}

int main()
{
    const auto testData = getTestData();
    runTestCases(testData);

    return 0;
}
