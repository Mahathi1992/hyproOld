#include "benchmarkBox.h"

namespace benchmark{
namespace box
{
    Results<int> intersectHalfspaces(const Settings& settings) {
        Results<int> ress;
        hypro::Box<::benchmark::Number> box;
        // benchmark against PPL
        #ifdef HYPRO_USE_PPL
        using pplItv = Parma_Polyhedra_Library::Interval<double,Parma_Polyhedra_Library::Interval_Info_Null<::benchmark::Double_Interval_Policy>>;
        using pplbox = Parma_Polyhedra_Library::Box<pplItv>;
        #endif
        box.insert(carl::Interval<::benchmark::Number>(-1,1));

        // initialize random number generator
        mt19937 generator;
        std::uniform_int_distribution<int> dist = std::uniform_int_distribution<int>(0,10);

        // iterate over dimensions
        for(std::size_t d = 1; d < settings.maxDimension; ++d) {
            // create instances
            std::vector<hypro::Halfspace<::benchmark::Number>> hsps;
            #ifdef HYPRO_USE_PPL
            std::vector<Parma_Polyhedra_Library::Constraint> pplHsps;
            #endif
            Timer creationTimer;
            for(std::size_t i = 0; i < settings.iterations; ++i) {
                hypro::vector_t<::benchmark::Number> normal = hypro::vector_t<::benchmark::Number>(d);
                for(std::size_t id = 0; id < d; ++id) {
                    normal(id) = dist(generator);
                }
                hsps.emplace_back(hypro::Halfspace<::benchmark::Number>(normal,0));
                // create same constraint for PPL.
                #ifdef HYPRO_USE_PPL
                Parma_Polyhedra_Library::Constraint c;
                Parma_Polyhedra_Library::Linear_Expression e;
                for (dimension_type i = 0; i < d; ++i )
                    e += hsps.back().normal()(i) * Variable(i);
                e += -hsps.back().offset();
                c = e <= 0;
                pplHsps.emplace_back(c);
                #endif
            }
            auto creationTime = creationTimer.elapsed();
            std::cout << "Dimension " << d << ": Creation took " << creationTime.count() << " sec." << std::endl;
            ress.mCreationTime += creationTime;

            // run instances
            Timer runTimerHyPro;
            for(std::size_t i = 0; i < settings.iterations; ++i) {
                box.intersectHalfspace(hsps[i]);
            }
            auto runningTime = runTimerHyPro.elapsed();
            std::cout << "Dimension " << d << ":  Running took " << runningTime.count() << " sec." << std::endl;

            #ifdef HYPRO_USE_PPL
            std::chrono::duration<double> pplRT = std::chrono::duration<double>::zero();
            for(std::size_t i = 0; i < settings.iterations; ++i) {
                // construct fresh box
                pplbox b = pplbox(d);
                for(Parma_Polyhedra_Library::dimension_type id = 0; id < d; ++id) {
                    pplItv itv;
                    itv.lower() = -1;
                    itv.upper() = 1;
                    b.set_interval(Parma_Polyhedra_Library::Variable(id),itv);
                }
                Timer runTimerPPL;
                b.refine_with_constraint(pplHsps[i]);
                pplRT += runTimerPPL.elapsed();
            }
            std::cout << "Dimension " << d << ":  Running took " << pplRT.count() << " sec (PPL)." << std::endl;
            #endif

            ress.mRunningTime += runningTime;

            // prepare next run
            box.insert(carl::Interval<::benchmark::Number>(-1,1));
            #ifdef HYPRO_USE_PPL

            #endif
        }
        return ress;
    }

    Results<int> run(const Settings& settings) {
        Timer totalRunningTime;
        Results<int> ress;

        // half-space intersection
        auto tmp = intersectHalfspaces(settings);
        ress.insert(ress.end(), tmp.begin(), tmp.end());       

        std::cout << "Box benchmarking took " << totalRunningTime.elapsedMs() << " sec in total." << std::endl;

        return ress;
    }
} // box    
} // benchmark
