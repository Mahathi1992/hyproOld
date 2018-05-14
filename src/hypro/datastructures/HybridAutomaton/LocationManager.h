/**
 * @author Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 */

#pragma once

#include "Location.h"
#include <carl/util/Singleton.h>
#include "../../types.h"

namespace hypro
{
template<typename Number>
class LocationManager : public carl::Singleton<LocationManager<Number>>
{
    friend carl::Singleton<LocationManager<Number>>;

  private:
    //std::map<unsigned, Location<Number>*> mLocations;
    //std::map<Location<Number>*, unsigned> mIds;
    std::multimap<unsigned, Location<Number>*> mLocations;
    std::multimap<Location<Number>*, unsigned> mIds;
    unsigned mId;

  protected:
    /**
     * Default constructor and destructor
     */
    LocationManager() : mId(0) {}
  public:
    //LocationManager() : mId(0) {}
    ~LocationManager() {
      std::cout << "locMan verabschiedet sich\n";
    	while(!mLocations.empty()) {
    		Location<Number>* toDelete = mLocations.begin()->second;
    		mLocations.erase(mLocations.begin());
    		delete toDelete;
    	}
    }

    Location<Number>* create();
    Location<Number>* create(const Location<Number>* _loc);
    Location<Number>* create(const std::unique_ptr<Location<Number>>& _loc);
    Location<Number>* create(const matrix_t<Number> _mat);
    Location<Number>* create(const matrix_t<Number> _mat, const typename Location<Number>::transitionSet _trans, const Condition<Number>& _inv);
    Location<Number>* create(const matrix_t<Number> _mat, const typename Location<Number>::transitionSet _trans, const Condition<Number>& _inv,
                     const matrix_t<Number> _extInputMat);

    unsigned id(Location<Number>* _loc) const;
    //Returns last inserted Location with given id
    Location<Number>* location(unsigned _id) const;
    Location<Number>* location(std::string name) const;
    std::vector<Location<Number>*> locations(unsigned _id) const;
    std::vector<Location<Number>*> locations(std::string name) const;

    void erase(unsigned _id);
};

}  // namespace hydra

#include "LocationManager.tpp"
