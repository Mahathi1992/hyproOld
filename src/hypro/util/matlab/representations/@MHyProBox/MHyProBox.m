classdef MHyProBox < MHyProGeometricObject
    methods (Access = public)
        
        % Create a HyPro box
        function obj = MHyProBox(varargin) 
            obj.Type = 0;
            if nargin == 0
                % Call default constructor
                obj.Handle = MHyPro(0, 'new_empty');
            elseif nargin == 1
                if isa(varargin{1}, 'uint64')
                    % This constructor is needed in case HyPro returns
                    % a new box
                    obj.Handle = varargin{1};
                elseif isa(varargin{1}, 'MHyProBox')
                    % Call copy constructor
                    obj.Handle = MHyPro(0, 'copy', varargin{1}.Handle);
                elseif ismatrix(varargin{1})
                    obj.Handle = MHyPro(0, 'new_matrix', varargin{1});
                else
                    error('MHyProBox - Constructor: Wrong type of argument.');
                end
            elseif nargin == 2
                if strcmp(varargin{1}, 'points') && ismatrix(varargin{2})
                    obj.Handle = MHyPro(0, 'new_points', varargin{2});
                elseif strcmp(varargin{1}, 'interval') && areIntervals(varargin{2})
                    obj.Handle = MHyPro(0, 'new_interval', varargin{2});
                elseif strcmp(varargin{1}, 'intervals') && areIntervals(varargin{2})
                    obj.Handle = MHyPro(0, 'new_intervals', varargin{2});
                elseif ismatrix(varargin{1}) && isvector(varargin{2}) && (size(varargin{1}, 2)== size(varargin{2},1))
                    obj.Handle = MHyPro(0, 'new_mat_vec', varargin{1}, varargin{2});
                else
                    error('MHyProBox - Constructor: Wrong arguments.');
                end
            else
                error('MHyProBox - Constructor: wrong arguments.');
            end
        end
                        
        function out = empty(obj,dim)
            if mod(dim, 1) == 0
                ptr = MHyPro(0, 'empty', obj.Handle, dim);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - empty: Wrong type of input argument.');
            end
        end
       
        function [containment, out] = satisfiesHalfspace(obj, normal, offset)
            if isvector(normal) && isreal(offset)
                [containment, ptr] = MHyPro(0, 'satisfiesHalfspace', obj.Handle, normal, offset);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - satisfiesHalfspace: Wrong type of input argument.');
            end
        end
        
        function [containment, out] = satisfiesHalfspaces(obj, mat, vec)
            if ismatrix(mat) && isvector(vec) && size(mat,2) == size(vec,1)
                [containment, ptr] = MHyPro(0, 'satisfiesHalfspace', obj.Handle, mat, vec);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - satisfiesHalfspaces: Wrong type of input argument.');
            end
        end
        
        function out = project(obj, dim)
            max = obj.dimension();
            if isvector(dim) && size(dim, 1) <= max 
                ptr = MHyPro(0, 'project', obj.Handle, dim);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - project: Wrong type of input argument.');
            end
        end
        
        function out = linearTransformation(obj, mat)
            if ismatrix(mat)
                ptr = MHyPro(0, 'linearTransformation', obj.Handle, mat);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - linearTransformation: Wrong type of input argument.');
            end
        end
        
        function out = affineTransformation(obj, mat, vec)
            if ismatrix(mat) && isvector(vec) && size(mat,2) == size(vec,1)
                ptr = MHyPro(0, 'affineTransformation', obj.Handle, mat, vec);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - affineTransformation: Wrong type of input argument.');
            end 
        end
        
        function out = plus(obj, rhs)
            if isa(rhs, 'MHyProBox')
                ptr = MHyPro(0, 'minkowskiSum', obj.Handle, rhs.Handle);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - minkowskiSum: Wrong type of input argument.');
            end
        end
        
        function out = intersectHalfspaces(obj, mat, vec)
            if ismatrix(mat) && isvector(vec) && size(mat,2) == size(vec,1)
                ptr = MHyPro(0, 'intersectHalfspaces', obj.Handle, mat, vec);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - intersectHalfspaces: Wrong type of input argument.');
            end
        end
        
        function out = intersectHalfspace(obj, nor, off)
            if isvector(nor) && isreal(off)
                ptr = MHyPro(0, 'intersectHalfspace', obj.Handle, nor, off);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - intersectHalfspace: Wrong type of input argument.');
            end
        end

        function out = contains(obj, arg)
            if isreal(arg)
                out = MHyPro(0, 'contains_point', obj.Handle, arg);
            elseif isa(arg, 'MHyProBox')
                out = MHyPro(0, 'contains_set', obj.Handle, arg.Handle);
            else
                error('MHyProBox - contains: Wrong type of input argument.');
            end
        end
        
        function out = unite(obj, rhs)
            if isa(rhs, 'MHyProBox')
                ptr = MHyPro(0, 'unite', obj.Handle, rhs.Handle);
                out = MHyProBox(ptr);
            elseif iscell(rhs)
                objects = cell(1, size(rhs, 2));
                for i = 1:length(rhs)
                    if isa(rhs{i}, 'MHyProBox')
                        objects{i} = rhs{i}.Handle;
                    else
                        error('MHyProBox - unite: Wrong type of input argument.');
                    end
                end
                ptr = MHyPro(0, 'unite_objects', obj.Handle, objects);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - unite: Wrong type of input argument.');
            end
        end
 
%         function plot(obj, dims)
%             isempty = MHyPro(obj.Type, 'isEmpty', obj.Handle);
%             if isempty
%                 warning('MHyProBox - plot: It is not possible to plot an empty object.');
%             else
%                 intervals = MHyPro(0, 'intervals', obj.Handle);
%                 inter_1 = intervals(dims(1),:);
%                 inter_2 = intervals(dims(2),:);
%                 [~, dimx] = size(inter_1);
%                 ver = zeros(dimx*2, 2);
%                 counter = 0;
% 
%                 for i = 1:(dimx)
%                     for j = 1:(dimx)
%                         ver(j+counter,1) = inter_1(i);
%                         ver(j+counter,2) = inter_2(j);
%                     end
%                     counter = counter + dimx;
%                 end
% 
%                 % Sort the vertices clockwise
%                 ver_x = ver(:,1).';
%                 ver_y = ver(:,2).';
%                 cx = mean(ver_x);
%                 cy = mean(ver_y);
%                 a = atan2(ver_y - cy, ver_x - cx);
%                 [~, order] = sort(a);
%                 ver_x = ver_x(order);
%                 ver_y = ver_y(order);
% 
%                 pgon = polyshape(ver_x, ver_y);
%                 plot(pgon);                
%             end
%         end

        function out = reduceNumberRepresentation(obj)
            ptr = MHyPro(0, 'reduceNumberRepresentation', obj.Handle);
            out = MHyProBox(ptr);
        end

        function out = intervals(obj)
            out = MHyPro(0, 'intervals', obj.Handle);
        end
        
        function out = limits(obj)
            out = MHyPro(0, 'limits', obj.Handle);
        end
        
        function [containment, box] = constraints(obj)
            [containment, box] = MHyPro(obj.Type, 'constraints', obj.Handle);
        end
        
        function insert(obj, inter)
            if areIntervals(inter)
                MHyPro(0, 'insert', obj.Handle, inter);
            else
                error('MHyProBox - insert: Wrong type of argument.');
            end
        end
        
        function out = interval(obj, dim)
            if mod(dim, 1) == 0 && dim >= 1
                out = MHyPro(0, 'interval', obj.Handle, dim);
            else
                error('MHyProBox - interval: Wrong type of argument.');
            end
        end
        
        function out = at(obj, dim)
            if mod(dim, 1) == 0
                out = MHyPro(0, 'at', obj.Handle, dim);
            else
                error('MHyProBox - at: Wrong type of argument.');
            end
        end
        
        function out = isSymmetric(obj)
            out = MHyPro(0, 'isSymmetric', obj.Handle);
        end
        
        function out = max(obj)
            out = MHyPro(0, 'max', obj.Handle);
        end
        
        function out = min(obj)
            out = MHyPro(0, 'min', obj.Handle);
        end
        
        function out = supremum(obj)
            if obj.Type == 0 || obj.Type == 3
                out = MHyPro(obj.Type, 'supremum', obj.Handle);
            else
                error('MHyProObject - supremum: Not allowed for this type of HyProObject.');
            end
        end
        
        function out = evaluate(obj, vec, dir)
            if obj.Type == 0 && isreal(vec) && islogical(dir)
                out = MHyPro(obj.Type, 'evaluate', obj.Handle, vec, dir);
            else
                error('MHyProObject - evaluate: Not allowed for this type of HyProObject or wrong type of argument.');
            end
        end
        
        function out = mtimes(obj, scalar)
            if isreal(scalar)
                ptr = MHyPro(0, '*', obj.Handle, scalar);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - scale: Wrong type of argument.');
            end
        end
        
        function out = makeSymmetric(obj)
            ptr = MHyPro(0, 'makeSymmetric', obj.Handle);
            out = MHyProBox(ptr);
        end
        
        function out = minkowskiDecomposition(obj, rhs)
            if isa(rhs, 'MHyProBox')
                ptr = MHyPro(obj.Type, 'minkowskiDecomposition', obj.Handle, rhs.Handle);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - minkowskiDecomposition: Wrong type of argument.');
            end
        end
        
        function out = intersect(obj, rhs)
            if isa(rhs, 'MHyProBox') 
                ptr = MHyPro(0, 'intersect', obj.Handle, rhs.Handle);
                out = MHyProBox(ptr);
            else
                error('MHyProBox - intersect: Wrong type of argument.');
            end
        end

    end
end