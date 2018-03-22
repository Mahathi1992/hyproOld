#include "Plotter.h"

namespace hypro {

template <typename Number>
Plotter<Number>::~Plotter() {}

template <typename Number>
void Plotter<Number>::setFilename( const std::string &_filename ) {
	mFilename = _filename;
	mSettings.name = _filename;
}

template <typename Number>
void Plotter<Number>::updateSettings( const plotting::gnuplotSettings& _settings ) {
	mSettings = _settings;
}

template<typename Number>
const plotting::gnuplotSettings& Plotter<Number>::settings() const {
	return mSettings;
}

template<typename Number>
plotting::gnuplotSettings& Plotter<Number>::rSettings() {
	return mSettings;
}

template <typename Number>
void Plotter<Number>::plot2d() const {
	mOutfile.open( mFilename + "_pdf.plt" );

	if ( (!mObjects.empty() && !mObjects.begin()->second.empty()) || !mPoints.empty() ) {

		// preamble
		mOutfile << "# settings\n";
		mOutfile << "set title \"" << mSettings.name << "\"\n";
		if(mSettings.keepAspectRatio) {
			mOutfile << "set size square\n";
		}
		mOutfile << "set term pdf font ',10'\n";
		mOutfile << "set output \"" << mFilename << ".pdf\n";

		writeGnuplot();
	}
	std::cout <<"Plotted to " << mFilename << "_pdf.plt" << std::endl;
	mOutfile.close();
}

template<typename Number>
void Plotter<Number>::plotTex() const {
	mOutfile.open( mFilename + "_tex.plt" );
	if ( (!mObjects.empty() && !mObjects.begin()->second.empty()) || !mPoints.empty() ) {

		// preamble
		mOutfile << "# settings\n";
		mOutfile << "set title \"" << mSettings.name << "\"\n";
		if(mSettings.keepAspectRatio) {
			mOutfile << "set size square\n";
		}
		mOutfile << "set terminal lua tikz latex standalone color header \"\\\\usepackage[T1]{fontenc}\\\n\\\\usepackage{mathptmx}\\\n\\\\usepackage{helvet}\\\n\\\\usepackage{amsmath}\"\n";
		mOutfile << "set output \"" << mFilename << ".tex\" \n";

		writeGnuplot();
	}

	std::cout << std::endl << "Plotted to " << mFilename << "_tex.plt" << std::endl;
	mOutfile.close();
}

template<typename Number>
void Plotter<Number>::plotGen() const {
	prepareObjects();

	mOutfile.open( mFilename + ".gen" );

	if(!mVectors.empty()){
		// TODO: implement gen file plotting for vectors.
	}

	if ( !mObjects.empty() || !mObjects.begin()->second.empty() || !mPoints.empty() ) {
		for ( auto objectIt = mObjects.begin(); objectIt != mObjects.end(); ++objectIt ) {
			if(objectIt->second.size() > 0){
				for ( unsigned pointIndex = 0; pointIndex < objectIt->second.size(); ++pointIndex ) {
					assert( objectIt->second[pointIndex].dimension() == 2 );
					if ( objectIt->second[pointIndex].dimension() == 0 ) {
						continue;
					}
					mOutfile << carl::toDouble( objectIt->second[pointIndex].at( 0 ) );
					for ( unsigned d = 1; d < objectIt->second[pointIndex].dimension(); ++d ) {
						mOutfile << " " << carl::toDouble( objectIt->second[pointIndex].at( d ) );
					}
					mOutfile << "\n";
				}
				mOutfile << carl::toDouble( objectIt->second[0].at( 0 ) );
				for ( unsigned d = 1; d < objectIt->second[0].dimension(); ++d ) {
					mOutfile << " " << carl::toDouble( objectIt->second[0].at( d ) );
				}
				mOutfile << "\n";
			}
			mOutfile << "\n\n\n";
		}

		// create plane functions
		if(!mPlanes.empty()){
			// TODO: implement.
		}

		if(!mPoints.empty()){
			// TODO: implement.
		}
	}
	mOutfile.close();
	std::cout << std::endl << "Plotted to " << mFilename << ".gen" << std::endl;
}

template<typename Number>
void Plotter<Number>::plotEps() const {
	mOutfile.open( mFilename + "_eps.plt" );
	if ( (!mObjects.empty() && !mObjects.begin()->second.empty()) || !mPoints.empty() ) {

		// preamble
		mOutfile << "# settings\n";
		mOutfile << "set title \"" << mSettings.name << "\"\n";
		if(mSettings.keepAspectRatio) {
			mOutfile << "set size square\n";
		}
		mOutfile << "set terminal postscript eps  enhanced color font 'Helvetica,20' linewidth 2\n";
		mOutfile << "set output \"" << mFilename << ".eps\" \n";

		writeGnuplot();
	}

	std::cout << std::endl << "Plotted to " << mFilename << "_eps.plt" << std::endl;
	mOutfile.close();
}

template <typename Number>
unsigned Plotter<Number>::addObject( const std::vector<Point<Number>> &_points ) {
	TRACE("hypro.plotter","");
	// reduce dimensions
	if(!_points.empty()){
		assert(_points.begin()->dimension() == 2);
		// initialize limits
		if(mObjects.empty() && mPoints.empty()){
			mLimits.first = _points.begin()->rawCoordinates();
			mLimits.second = _points.begin()->rawCoordinates();
		}
		// update limits
		for(const auto& point : _points) {
			assert(point.dimension() == 2);
			for(unsigned d = 0; d < mLimits.first.rows(); ++d){
				mLimits.first(d) = mLimits.first(d) > point.rawCoordinates()(d) ? point.rawCoordinates()(d) : mLimits.first(d);
				mLimits.second(d) = mLimits.second(d) < point.rawCoordinates()(d) ? point.rawCoordinates()(d) : mLimits.second(d);
			}
		}

		mObjects.insert( std::make_pair( mId, _points ) );
		mId++;
		return (mId-1);
	}
	return 0;
}

template <typename Number>
unsigned Plotter<Number>::addObject( const std::vector<std::vector<Point<Number>>> &_points ) {
	for ( const auto &part : _points ) {
		addObject(part);
		--mId;
	}
	mId++;
	return mId-1;
}

template<typename Number>
unsigned Plotter<Number>::addObject( const std::vector<Halfspace<Number>>& _planes ) {
	mPlanes.insert( std::make_pair( mId++, _planes ) );
	return mId;
}

template<typename Number>
unsigned Plotter<Number>::addObject( const Halfspace<Number>& _plane ) {
	assert(_plane.dimension() == 2);
	std::vector<Halfspace<Number>> tmp;
	tmp.push_back(_plane);
	mPlanes.insert( std::make_pair( mId++, tmp ) );
	return mId;
}

template<typename Number>
unsigned Plotter<Number>::addPoint( const Point<Number>& _point ) {
	assert(_point.dimension() == 2);
	// initialize limits
	if(mObjects.empty() && mPoints.empty()){
		mLimits.first = _point.rawCoordinates();
		mLimits.second = _point.rawCoordinates();
	}
	// update limits
	for(unsigned d = 0; d < mLimits.first.rows(); ++d){
		mLimits.first(d) = mLimits.first(d) > _point.rawCoordinates()(d) ? _point.rawCoordinates()(d) : mLimits.first(d);
		mLimits.second(d) = mLimits.second(d) < _point.rawCoordinates()(d) ? _point.rawCoordinates()(d) : mLimits.second(d);
	}

	mPoints.insert( std::make_pair( mId, _point ) );
	return mId++;
}

template<typename Number>
unsigned Plotter<Number>::addPoints( const std::vector<Point<Number>>& _points ) {
	for(const auto& p : _points){
		assert(p.dimension() == 2);
		addPoint(p);
		--mId;
	}
	mId++;
	return mId-1;
}

template<typename Number>
void Plotter<Number>::addVector( const vector_t<Number>& _vector ) {
	mVectors.insert( std::make_pair(mId++, _vector) );
}

template <typename Number>
void Plotter<Number>::setObjectColor( unsigned _id, const std::size_t _color ) {
	if ( _color != 0 && mObjects.find( _id ) != mObjects.end() ) {
		mObjectColors[_id] = _color;
	} else if ( _color != 0 && mPlanes.find( _id ) != mPlanes.end() ) {
		mObjectColors[_id] = _color;
	}
}

template<typename Number>
void Plotter<Number>::clear() {
	mLastDimensions = std::make_pair(-1,-1);
	mId = 1;
	mObjects.clear();
	mPlanes.clear();
	mPoints.clear();
	mVectors.clear();
	mObjectColors.clear();
}

template <typename Number>
void Plotter<Number>::init( const std::string &_filename ) {
	mOutfile.open( _filename );
}

template <typename Number>
void Plotter<Number>::writeGnuplot() const {
	prepareObjects();
	TRACE("hypro.plotter", "Prepared objects.");

	if(!mVectors.empty()){
		mOutfile << "# plotting vectors normalized to length 1\n";
		unsigned arrowIndex = 0;
		for(auto& vector : mVectors) {
			vector_t<Number> normalized = vector.second/norm(vector.second);
			mOutfile << "set arrow " << arrowIndex++ << " from 0,0 to " << normalized(0) << "," << normalized(1) << "\n";
		}
		mOutfile << "\n";
	}


	if ( (!mObjects.empty() && !mObjects.begin()->second.empty()) || !mPoints.empty() ) {
		TRACE("hypro.plotter","Start plotting objects.");
		// set object
		vector_t<Number> min = vector_t<Number>(2);
		vector_t<Number> max = vector_t<Number>(2);
		min(0) = mLimits.first(0);
		min(1) = mLimits.first(1);
		max(0) = mLimits.second(0);
		max(1) = mLimits.second(1);
		// extend ranges
		std::map<unsigned, carl::Interval<double>> ranges;
		for ( unsigned d = 0; d < min.rows(); ++d ) {
			double rangeExt = carl::toDouble( ( carl::toDouble(max( d )) - carl::toDouble(min( d )) ) * 0.1 );
			if(rangeExt != 0){
				ranges[d] = carl::Interval<double>(carl::toDouble(min( d )) - rangeExt, carl::toDouble(max( d )) + rangeExt );
			} else{
				rangeExt = min(d) == 0 ? 0.1 : carl::toDouble(carl::toDouble(min( d ))* 0.1);
				ranges[d] = carl::Interval<double>(carl::toDouble(min( d )) - rangeExt, carl::toDouble(max( d )) + rangeExt );
			}
		}

		// preamble

		mOutfile << "set xtics autofreq\n";
		mOutfile << "set ytics autofreq\n";

		if(!mSettings.plain) {
			if(mSettings.grid) {
				mOutfile << "set grid back\n";
			}
			if(mSettings.axes) {
				mOutfile << "# axis settings\n";
				mOutfile << "set xzeroaxis\n";
				mOutfile << "set zeroaxis\n";
	            mOutfile << "set xtics axis\n";
				mOutfile << "set yzeroaxis\n";
	            mOutfile << "set ytics axis\n";
			}
		} else {
			mOutfile << "unset border\nunset xtics\nunset ytics\n";
		}


		mOutfile << "set xrange ["<< ranges[0].lower() << ":" << ranges[0].upper() << "] \n";
		mOutfile << "set yrange ["<< ranges[1].lower() << ":" << ranges[1].upper() << "] \n";

		unsigned objectCount = 1;
		unsigned currId = 0;
		unsigned tmpId = 0;
		mOutfile << "\n# plotting sets\n";

		//std::cout << "Start plotting sets." << std::endl;

		for ( auto objectIt = mObjects.begin(); objectIt != mObjects.end(); ++objectIt ) {
			if ( currId != objectIt->first ) {
				currId = objectIt->first;
				tmpId++;
				INFO("hypro.plotter","Plotting object " << tmpId << "/" << (mObjects.size() + mPoints.size() + mPlanes.size()));
			}
			if(objectIt->second.size() > 0){
				mOutfile << "set object " << std::dec << objectCount << " polygon from \\\n";
				for ( unsigned pointIndex = 0; pointIndex < objectIt->second.size(); ++pointIndex ) {
					assert( objectIt->second[pointIndex].dimension() == 2 );
					if ( objectIt->second[pointIndex].dimension() == 0 ) {
						continue;
					}
					mOutfile << "  " << carl::toDouble( objectIt->second[pointIndex].at( 0 ) );
					for ( unsigned d = 1; d < objectIt->second[pointIndex].dimension(); ++d ) {
						mOutfile << ", " << carl::toDouble( objectIt->second[pointIndex].at( d ) );
					}
					mOutfile << " to \\\n";
				}
				// assert(objectIt->objectIt->size()-1].dimension() <= 2); // TODO:
				// Project to 2d	TODO: REINSERT ASSERTION
				// std::cout << carl::toDouble(objectIt->0].at(0)) << std::endl;
				mOutfile << "  " << carl::toDouble( objectIt->second[0].at( 0 ) );
				for ( unsigned d = 1; d < objectIt->second[0].dimension(); ++d ) {
					mOutfile << ", " << carl::toDouble( objectIt->second[0].at( d ) );
				}

				// color lookup
				auto color = mSettings.color;
				//std::cout << "Lookup color for object " << objectIt->first << std::endl;
				if ( mObjectColors.find( objectIt->first ) != mObjectColors.end() ) {
					color = mObjectColors.at( objectIt->first );
					//std::cout << "Found" << std::endl;
				}

				if ( mSettings.fill )
					mOutfile << " front fs transparent solid 0.75 fc rgb '#" << std::hex << color << "' lw " << mSettings.linewidth << "\n";
				else
					mOutfile << " front fs empty border lc rgb '#" << std::hex << color << "' lw " << mSettings.linewidth << "\n";

				if(mSettings.cummulative){
					mOutfile << "\nplot ";
					for ( unsigned d = 0; d < min.rows(); ++d ) {
						mOutfile << "[" << ranges[d].lower() << ":" << ranges[d].upper() << "] ";
					}
					mOutfile << "NaN notitle \n";
				}
			}
			++objectCount;
		}

		//std::cout << "Done plotting sets." << std::endl;

		if(mPlanes.empty() && mPoints.empty()){
			mOutfile << "plot - NaN notitle\n";
		}


		// create plane functions
		int index = 1;
		if(!mPlanes.empty()){
			mOutfile << "\n";
			mOutfile << "# plotting Halfspaces\n";
			for( const auto& planePair : mPlanes ) {
				for( const auto& plane : planePair.second ) {
					//std::cout << "Plot plane " << plane << std::endl;
					assert(plane.dimension() == 2);
					vector_t<Number> normal = plane.normal();
					if(normal(1) == Number(0)){
						mOutfile << "set arrow from " << carl::toDouble(Number(plane.offset()/normal(0))) <<",graph(0,0) to " << carl::toDouble(Number(plane.offset()/normal(0))) << ",graph(1,1) nohead\n";
					} else {
						mOutfile << "f_" << index << "(x) = " << carl::toDouble(Number(-normal(0)/normal(1))) << "*x";
						double off = carl::toDouble(Number(plane.offset()/normal(1)));
						if(off >= 0)
							mOutfile << "+";

						mOutfile << off << "\n";
						++index;
					}
				}
			}

		}

		if(!mPoints.empty()){
			mOutfile << "# plotting points\n";
			mOutfile << "set multiplot\n";
			mOutfile << "unset key\n";
			mOutfile << "set pointsize " << mSettings.pointSize << "\n";
			mOutfile << "set style line 1 lc rgb '#" << std::hex << mSettings.color << "'\n";
			mOutfile << "plot ";
			mOutfile << "'-' w p ls 1";
			for(unsigned pos = 1; pos < mPoints.size(); ++pos){
				mOutfile << ", '-' w p ls 1";
			}
			mOutfile << "\n";
		}
		for(auto pointIt = mPoints.begin(); pointIt != mPoints.end(); ++pointIt ){
			mOutfile << carl::toDouble(pointIt->second.at(0)) << " " << carl::toDouble(pointIt->second.at(1)) << "\n";
			mOutfile << "e\n";
		}
		mOutfile << "\n";

		if(!mPlanes.empty()){
			if(mPoints.empty()){
				mOutfile << "set multiplot \n";
				mOutfile << "unset key\n";
			}
			//plot all planes
			auto color = mSettings.color;
			mOutfile << "set style line 1 linecolor rgb '#" << std::hex << color << "' \n";
			while(index > 1){
				--index;
				mOutfile << "plot f_" << index << "(x) with lines linestyle 1\n";
			}
		}

		if(!mPlanes.empty() || !mPoints.empty())
			mOutfile << "\nunset multiplot\n";
	}
}


template <typename Number>
std::vector<Point<Number>> Plotter<Number>::grahamScan( const std::vector<Point<Number>> &_points ) {
	std::vector<Point<Number>> res;
	if(_points.size() < 3){
		return _points;
	}

	// initialize -> find minimum Point
	Point<Number> min = _points[0];
	std::map<Number, Point<Number>> sortedPoints;
	for ( const auto &point : _points ) {
		assert( point.dimension() == 2 );
		if ( point < min ) {
			min = point;
		}
	}

	// std::cout << "Minimum: " << min.rawCoordinates().transpose() <<
	// std::endl;

	// sort Points according to polar angle -> we have to insert manually
	// (because of double imprecision)
	for ( const auto &point : _points ) {
		if ( point != min ) {
			// std::cout << "Try to insert " << point << std::endl;
			Number angle = point.polarCoordinates( min ).at( 1 );
			// std::cout << "Computed polar angle: " << angle << std::endl;
			if ( sortedPoints.empty() ) {
				// std::cout << "points empty, simply insert." << std::endl;
				sortedPoints.insert( std::make_pair( angle, point ) );
			} else {
				// std::cout << "Compare." << std::endl;
				for ( auto pos = sortedPoints.begin(); pos != sortedPoints.end(); ) {
					// if equal, take the one with bigger radial component
					// std::cout << "Consider " << pos->second << ", angle: ";
					Number newAngle = pos->second.polarCoordinates( min ).at( 1 );
					// std::cout << newAngle << std::endl;
					if ( angle == newAngle ) {
						// if equal, compare radial coordinate (distance)
						// std::cout << "AlmostEqual2sComplement" << std::endl;
						if ( pos->second.polarCoordinates( min )[0] < point.polarCoordinates( min )[0] ) {
							// std::cout << "sortedPoints erase " << pos->second <<
							// std::endl;
							pos = sortedPoints.erase( pos );
							sortedPoints.insert( std::make_pair( angle, point ) );
						}
						break;
					}
					// we assume to be sorted, so check all angles, which are smaller or
					// equal for equality -
					// afterwards simply insert
					else if ( angle > newAngle ) {  // not equal and smaller -> continue
													// search (at end, simply insert)
						++pos;
						if ( pos == sortedPoints.end() ) {
							sortedPoints.insert( std::make_pair( angle, point ) );
							break;
						}
					} else {  // larger and not equal until this point -> simply insert
						sortedPoints.insert( std::make_pair( angle, point ) );
						break;
					}
				}
				// std::cout << "End Compare." << std::endl;
			}
		}
	}

	// for(const auto& pair : sortedPoints)
	//	std::cout << "sorted: " << pair.first << ", " <<
	// pair.second.rawCoordinates().transpose() << std::endl;

	// prepare stack -> initialize with 2 points
	if(sortedPoints.empty()) {
		res.emplace_back(min);
		return res;
	}
	assert( sortedPoints.size() >= 1 );
	std::stack<Point<Number>> stack;
	stack.push( min );
	stack.push( sortedPoints.begin()->second );
	sortedPoints.erase( sortedPoints.begin() );
	unsigned i = 0;
	unsigned n = sortedPoints.size();

	// main loop -> check the two topmost elements of the stack and one third,
	// new point
	while ( i < n ) {
		Point<Number> p1 = stack.top();
		stack.pop();
		Point<Number> p2 = stack.top();
		stack.pop();
		// std::cout << __func__ << ": " << p2.rawCoordinates().transpose() << "
		// -- " <<
		// p1.rawCoordinates().transpose() << " -- " <<
		// sortedPoints.begin()->second.rawCoordinates().transpose() <<
		// std::endl;
		if ( isLeftTurn( p2, p1, sortedPoints.begin()->second ) ) {
			// reinsert and add new point
			stack.push( p2 );
			stack.push( p1 );
			stack.push( sortedPoints.begin()->second );
			sortedPoints.erase( sortedPoints.begin() );
			++i;
		} else {
			// only reinsert second -> equal to removing the topmost object of the
			// stack
			// std::cout << "Drop " << p1.rawCoordinates().transpose() << std::endl;
			stack.push( p2 );
			if ( stack.size() < 2 ) {
				// in this case simply insert, as the stack has to contain at least 2
				// points
				stack.push( sortedPoints.begin()->second );
				sortedPoints.erase( sortedPoints.begin() );
				++i;
			}
		}
	}

	// write result
	while ( !stack.empty() ) {
		res.push_back( stack.top() );
		stack.pop();
	}

	return res;
}

template <typename Number>
bool Plotter<Number>::isLeftTurn( const Point<Number> &a, const Point<Number> &b, const Point<Number> &c ) {
	assert( a.dimension() == 2 );
	assert( b.dimension() == 2 );
	assert( c.dimension() == 2 );

	Number val = ( ( b.rawCoordinates()( 0 ) - a.rawCoordinates()( 0 ) ) *
				   ( c.rawCoordinates()( 1 ) - a.rawCoordinates()( 1 ) ) ) -
				 ( ( c.rawCoordinates()( 0 ) - a.rawCoordinates()( 0 ) ) *
				   ( b.rawCoordinates()( 1 ) - a.rawCoordinates()( 1 ) ) );
	// Number val = c.polarCoordinates(a,false)[1] -
	// b.polarCoordinates(a,false)[1];

	return ( val > 0 );
}

template<typename Number>
void Plotter<Number>::prepareObjects() const {
	// reduce and sort objects
	if(!mObjects.empty()){
		TRACE("hypro.plotter","Prepare " << mObjects.size() << " objects.");
		for(auto objIt = mObjects.begin(); objIt != mObjects.end(); ++objIt){
			// sort objects
			TRACE("hypro.plotter","Sort vertices of object " << objIt->first);
			objIt->second = grahamScan(objIt->second);
		}
	}
}

} // namespace hypro
