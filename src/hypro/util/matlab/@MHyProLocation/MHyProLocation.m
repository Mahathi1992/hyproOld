classdef MHyProLocation < handle
    
    properties (SetAccess = public, GetAccess = public)
        Handle
    end
    
    methods (Access = private)
        
        % Destructor
        function delete(obj)
            MHyPro('Location', 'delete', obj.Handle);
        end
        
        %Auxiliary functions
        function out = iscelloftype(cell, type)
            for i=(1:length(cell))
                elem = cell(i);
                if strcmp(elem.Type, type) == 0
                    out = 0;
                end
            end
            out = 1;
        end
    end
    
    methods (Access = public)
        
        %Constructor
         function obj = MHyProLocation(varargin)
            if strcmp(varargin{1},'Location')
                obj.Type = 'Location';
            else
                error('MHyProLocation - Constructor: Unknown type.');
            end 

            if nargin == 1
                % Call the constructor for empty flow
                obj.Handle = MHyPro('Location', 'new_empty');
            elseif nargin == 2
                if isa(varargin{2}, 'uint64')
                    % Call the copy constructor
                    obj.Handle = MHyPro('Location', 'copy', varargin{2});
                elseif ismatrix(varargin{2})
                    % Call the matrix constructor
                    obj.Handle = MHyPro('Location', 'new_mat', varargin{2});
                else
                    error('MHyProLocation - Constructor: Wrong type of at least one argument.');
                end
            elseif nargin == 3
                if ismatrix(varargin{2}) && isvector(varargin{3})
                    % Call the matrix-vector constructor for affine flow
                    obj.Handle = MHyPro('Location', 'new_mat_vec' ,varargin{2}, varargin{3});
                else
                    error('MHyProLocation - Constructor: Wrong type of at leat one argument.');
                end
            else
                error('MHyProLocation - Constructor: Wrong type of at leat one argument.');
            end
         end
        
        function out = getNumberFlow(obj)
            out = MHyPro('Location', 'getNumberFlow', obj.Handle);
        end
        
        function out = getLinearFlow(obj)
            out = MHyPro('Location', 'getLinearFlow', obj.Handle);
        end
        
        function out = getInvariant(obj)
            out = MHyPro('Location', 'getInvariant', obj.Handle);
        end
        
        function out = getTransitions(obj)
            out = MHyPro('Location', 'getTransitions', obj.Handle);
        end
        
        function out = getExternalInput(obj)
            out = MHyPro('Location', 'getExternalInput', obj.Handle);
        end
        
        function out = hasExternalInput(obj)
            out = MHyPro('Location', 'hasExternalInput', obj.Handle);
        end
        
        function out = hash(obj)
            out = MHyPro('Location', 'hash', obj.Handle);
        end
        
        function out = getName(obj)
            out = MHyPro('Location', 'getName', obj.Handle);
        end
        
        function out = dimension(obj)
            out = MHyPro('Location', 'dimension', obj.Handle);
        end
        
        function out = dimension_at(obj)
            out = MHyPro('Location', 'dimension_at', obj.Handle);
        end
        
        function setName(obj, name)
            if isStringScalar(name)
                MHyPro('Location', 'setName', obj.Handle, name);
            else
                error("MHyProLocation - setName: Wrong type of at leat one argument. Note: strings in MATLAB have to be written in "".");
            end
        end
        
        function setLinearFlow(obj, linFlow, pos)
            if isa(linFlow, "MHyProFlow") && strcmp(linFlow.Type, "linearFlow") && isdouble(pos)
                MHyPro('Location', 'setLinearFlow', obj.Handle, linFlow, pos);
            else
                error("MHyProLocation - setLinearFlow: Wrong type of at leat one argument.");
            end
        end
        
        function setLinearFlows(obj, linFlows)
            if iscelloftype(linFlows, 'linearFlow') 
                MHyPro('Location', 'setLinearFlows', obj.Handle, linFlows);
            else
                error("MHyProLocation - setLinearFlows: Wrong type of at leat one argument.");
            end
        end
           
        function setInvariant(obj, cond)
            if isa(cond, 'MHyProCondition') 
                MHyPro('Location', 'setInvariant', obj.Handle, cond);
            else
                error("MHyProLocation - setInvariant: Wrong type of at leat one argument.");
            end
        end
        
        function setExtInput(obj, ints)
            if iscell(ints) && size(ints, 1) == 2 
                MHyPro('Location', 'setExtInput', obj.Handle, ints);
            else
                error("MHyProLocation - setExtInput: Wrong type of at leat one argument.");
            end
        end
        
        function out = isComposedOf(obj)
            %TODO
            out = 0;
        end
        
        function out = getDotRepresentation(obj, vars)
            if isstring(vars)
                out = MHyPro('Location', 'getDotRepresentation', obj.Handle, vars);
            else
                error("MHyProLocation - getDotRepresentation: Wrong type of at leat one argument.");
            end
        end
        
        function out = decompose(obj)
            %TODO
        end
        
        function out = lt(obj, rhs)
            if isa(rhs, "MHyProLocation")
                out = MHyPro('Location', 'less', obj.Handle, rhs.Handle);
            else
                error("MHyProLocation - less: Wrong type of at leat one argument.");
            end
        end
        
        function out = eq(obj, rhs)
            if isa(rhs, "MHyProLocation")
                out = MHyPro('Location', 'equals', obj.Handle, rhs.Handle);
            else
                error("MHyProLocation - equals: Wrong type of at leat one argument.");
            end
        end
        
        function out = ne(obj, rhs)
            if isa(rhs, "MHyProLocation")
                out = MHyPro('Location', 'unequals', obj.Handle, rhs.Handle);
            else
                error("MHyProLocation - unequals: Wrong type of at leat one argument.");
            end
        end
        
    end
end