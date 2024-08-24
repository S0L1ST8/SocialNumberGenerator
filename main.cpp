#include <algorithm>
#include <array>
#include <cassert>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string_view>

enum class SexType { FEMALE, MALE };

class SocialNumberGenerator {
  protected:
    virtual int SexDigit(SexType sex) const noexcept = 0;
    virtual int NextRandom(unsigned year, unsigned month, unsigned day) = 0;
    virtual int ModuloValue() const noexcept = 0;

    SocialNumberGenerator(int min, int max) : ud_(min, max) {
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size>{};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        eng_.seed(seq);
    }

  public:
    std::string Generate(SexType sex, unsigned year, unsigned month, unsigned day) {
        std::stringstream snumber;

        snumber << SexDigit(sex);
        snumber << year << month << day;
        snumber << NextRandom(year, month, day);
        auto number = snumber.str();

        auto index = number.length();
        auto sum = std::accumulate(std::begin(number), std::end(number), 0u, [&index](unsigned int s, char c) {
            return s + static_cast<unsigned int>(index-- * (c - '0'));
        });
        auto rest = sum % ModuloValue();
        snumber << ModuloValue() - rest;

        return snumber.str();
    }

    virtual ~SocialNumberGenerator() = default;

  protected:
    std::map<unsigned, int> cache_;
    std::mt19937 eng_;
    std::uniform_int_distribution<> ud_;
};

class SoutheriaSocialNumberGenerator final : public SocialNumberGenerator {
  public:
    SoutheriaSocialNumberGenerator() : SocialNumberGenerator(1000, 9999) {}

  protected:
    int SexDigit(SexType sex) const noexcept override {
        if (sex == SexType::FEMALE) {
            return 1;
        }
        return 2;
    }

    int NextRandom(unsigned year, unsigned month, unsigned day) override {
        auto key = year * 1000 + month * 100 + day;
        while (true) {
            auto number = ud_(eng_);
            auto pos = cache_.find(number);
            if (pos == std::end(cache_)) {
                cache_[key] = number;
                return number;
            }
        }
    }

    int ModuloValue() const noexcept override {
        return 11;
    }
};

class NortheriaSocialNumberGenerator final : public SocialNumberGenerator {
  public:
    NortheriaSocialNumberGenerator() : SocialNumberGenerator(10000, 99999) {}

  protected:
    int SexDigit(SexType sex) const noexcept override {
        if (sex == SexType::FEMALE) {
            return 9;
        }
        return 7;
    }

    int NextRandom(unsigned year, unsigned month, unsigned day) override {
        auto key = year * 10000 + month * 100 + day;
        while (true) {
            auto number = ud_(eng_);
            auto pos = cache_.find(number);
            if (pos == std::end(cache_)) {
                cache_[key] = number;
                return number;
            }
        }
    }

    int ModuloValue() const noexcept override {
        return 11;
    }
};

class SocialNumberGeneratorFactory {
  public:
    SocialNumberGeneratorFactory() {
        generators_["northeria"] = std::make_unique<NortheriaSocialNumberGenerator>();
        generators_["southeria"] = std::make_unique<SoutheriaSocialNumberGenerator>();
    }

    SocialNumberGenerator* GetGenerator(std::string_view country) const {
        auto it = generators_.find(country.data());
        if (it != std::end(generators_)) {
            return it->second.get();
        }

        throw std::runtime_error("invalid country");
    }

  private:
    std::map<std::string, std::unique_ptr<SocialNumberGenerator>> generators_;
};

int main() {
    SocialNumberGeneratorFactory factory;

    auto sn = factory.GetGenerator("northeria")->Generate(SexType::FEMALE, 2022, 12, 25);
    auto ss = factory.GetGenerator("southeria")->Generate(SexType::MALE, 2023, 05, 17);
}
