function switchingSystem()

% vars: [x1,x2,x3,x4,x5]

% Create Automaton
automaton = MHyProHAutomaton();
dummy_reset = MHyProReset();
dummy_reset.setMatrix(eye(5));
dummy_reset.setVector([0; 0; 0; 0; 0]);

%-----------------------------------------------%
%              Location loc1
%-----------------------------------------------%
loc_1 = MHyProLocation();
loc_1.setName('loc1');

% Set flow:
% x1' = -0.8047*x1 + 8.7420*x2 - 2.4591*x3 - 8.2714*x4 - 1.8640*x5 + [-0.0845,0.0845]
% x2' = -8.6329*x1 - 0.5860*x2 - 2.1006*x3 + 3.6035*x4 - 1.8423*x5
% x3' = 2.4511*x1 + 2.2394*x2 - 0.7538*x3 - 3.6934*x4 + 2.4585*x5
% x4' = 8.3858*x1 - 3.1739*x2 + 3.7822*x3 - 0.6249*x4 + 1.8829*x5
% x5' = 1.8302*x1 + 1.9869*x2 - 2.4539*x3 - 1.7726*x4 - 0.7911*x5 + [-0.7342,0.7342]

flow_1 = [-0.8047 8.7420 -2.4591 -8.2714 -1.8640;...
    -8.6329 -0.5860 -2.1006 3.6035 -1.8423;...
    2.4511 2.2394 -0.7538 -3.6934 2.4585;...
    8.3858 -3.1739 3.7822 -0.6249 1.8829;...
    1.8302 1.9869 -2.4539 -1.7726 -0.7911];

loc_1.setFlow(flow_1);

% Set inv: x1 >= 3
inv_1 = MHyProCondition([-1 0 0 0 0], -3);
loc_1.setInvariant(inv_1);


%-----------------------------------------------%
%              Location loc2
%-----------------------------------------------%
loc_2 = MHyProLocation();
loc_2.setName('loc2');

% Set flow:
%     x1' = -0.8316*x1 + 8.7658*x2 - 2.4744*x3 - 8.2608*x4 - 1.9033*x5 + [-0.0845,0.0845]
%     x2' = -0.8316*x1 - 0.5860*x2 - 2.1006*x3 + 3.6035*x4 - 1.8423*x5
%     x3' = 2.4511*x1 + 2.2394*x2 - 0.7538*x3 - 3.6934*x4 + 2.4585*x5
%     x4' = 8.3858*x1 - 3.1739*x2 + 3.7822*x3 - 0.6249*x4 + 1.8829*x5
%     x5' = 1.5964*x1 + 2.1936*x2 - 2.5872*x3 - 1.6812*x4 - 1.1324*x5 + [-0.7342,0.7342]
flow_2 = [-0.8316 8.7658 -2.4744 -8.2608 -1.9033;...
        -0.8316 -0.5860 -2.1006 3.6035 -1.8423;...
        2.4511 2.2394 -0.7538 -3.6934 2.4585;...
        8.3858 -3.1739 3.7822 -0.6249 1.8829;...
        1.5964 2.1936 -2.5872 -1.6812 -1.1324];

loc_2.setFlow(flow_2);

% Set inv: x1 >= 2
inv_2 = MHyProCondition([-1 0 0 0 0], -2);
loc_2.setInvariant(inv_2);


%-----------------------------------------------%
%              Location loc3
%-----------------------------------------------%
loc_3 = MHyProLocation();
loc_3.setName('loc3');

% Set flow:
% x1' = -0.9275*x1 + 8.8628*x2 - 2.5428*x3 - 8.2329*x4 - 2.0324*x5 + [-0.0845,0.0845]
% x2' = -0.8316*x1 - 0.5860*x2 - 2.1006*x3 + 3.6035*x4 - 1.8423*x5
% x3' = 2.4511*x1 + 2.2394*x2 - 0.7538*x3 - 3.6934*x4 + 2.4585*x5
% x4' = 8.3858*x1 - 3.1739*x2 + 3.7822*x3 - 0.6249*x4 + 1.8829*x5
% x5' = 0.7635*x1 + 3.0357*x2 - 3.1814*x3 - 1.4388*x4 - 2.2538*x5 + [-0.7342,0.7342]

flow_3 = [-0.9275 8.8628 -2.5428 -8.2329 -2.0324;...
        -0.8316 -0.5860 -2.1006 3.6035 -1.8423;...
         2.4511 2.2394 -0.7538 -3.6934 2.4585;...
         8.3858 -3.1739 3.7822 -0.6249 1.8829;...
         0.7635 3.0357 -3.1814 -1.4388 -2.2538];


loc_3.setFlow(flow_3);
% Set inv: x1 >= 1
inv_3 = MHyProCondition([-1 0 0 0 0], -1);
loc_3.setInvariant(inv_3);


%-----------------------------------------------%
%              Location loc4
%-----------------------------------------------%
loc_4 = MHyProLocation();
loc_4.setName('loc4');

% Set flow:
%     x1' = -1.0145*x1 + 8.9701*x2 - 2.6207*x3 - 8.2199*x4 - 2.1469*x5 + [-0.0845,0.0845]
%     x2' = -0.8316*x1 - 0.5860*x2 - 2.1006*x3 + 3.6035*x4 - 1.8423*x5
%     x3' = 2.4511*x1 + 2.2394*x2 - 0.7538*x3 - 3.6934*x4 + 2.4585*x5
%     x4' = 8.3858*x1 - 3.1739*x2 + 3.7822*x3 - 0.6249*x4 + 1.8829*x5
%     x5' = 0.0076*x1 + 3.9682*x2 - 3.8578*x3 - 1.3253*x4 - 3.2477*x5 + [-0.7342,0.7342]

flow_4 = [-1.0145 8.9701 -2.6207 -8.2199 -2.1469;...
        -0.8316 -0.5860 -2.1006 3.6035 -1.8423;...
         2.4511 2.2394 -0.7538 -3.6934 2.4585;...
         8.3858 -3.1739 3.7822 -0.6249 1.8829;...
         0.0076 3.9682 -3.8578 -1.3253 -3.2477];


loc_4.setFlow(flow_4);
% Set inv: x1 >= 0
inv_4 = MHyProCondition([-1 0 0 0 0], 0);
loc_4.setInvariant(inv_4);


%-----------------------------------------------%
%              Location loc5
%-----------------------------------------------%
loc_5 = MHyProLocation();
loc_5.setName('loc5');

% Set flow:
%     x1' = -1.4021*x1 + 10.1647*x2 - 3.3937*x3 - 8.5139*x4 - 2.9602*x5 + [-0.0845,0.0845]
%     x2' = -0.8316*x1 - 0.5860*x2 - 2.1006*x3 + 3.6035*x4 - 1.8423*x5
%     x3' = 2.4511*x1 + 2.2394*x2 - 0.7538*x3 - 3.6934*x4 + 2.4585*x5
%     x4' = 8.3858*x1 - 3.1739*x2 + 3.7822*x3 - 0.6249*x4 + 1.8829*x5
%     x5' = -3.3585*x1 + 14.3426*x2 - 10.5703*x3 - 3.8785*x4 - 10.3111*x5 + [-0.7342,0.7342]
    
flow_5 = [-1.4021 10.1647 -3.3937 -8.5139 -2.9602;...
         -0.8316 -0.5860 -2.1006 3.6035 -1.8423;...
          2.4511 2.2394 -0.7538 -3.6934 2.4585;...
          8.3858 -3.1739 3.7822 -0.6249 1.8829;...
         -3.3585 14.3426 -10.5703 -3.8785 -10.3111];

loc_5.setFlow(flow_5);
% Set inv: x1 <= 1
inv_5 = MHyProCondition([1 0 0 0 0], 1);
loc_5.setInvariant(inv_5);

%-----------------------------------------------%
%              Add locations to HA
%-----------------------------------------------%

l1 = automaton.addLocation(loc_1);
l2 = automaton.addLocation(loc_2);
l3 = automaton.addLocation(loc_3);
l4 = automaton.addLocation(loc_4);
l5 = automaton.addLocation(loc_5);

%-----------------------------------------------%
%              loc1 --> loc2
%-----------------------------------------------%
tran1 = MHyProTransition();
% Set guard: x1 = 3 
guard1 = MHyProCondition();
guard1.setMatrix([1 0 0 0 0; -1 0 0 0 0]); 
guard1.setVector([3;-3]);

tran1.setAggregation(1);
tran1.setGuard(guard1);
tran1.setSource(l1);
tran1.setTarget(l2);
tran1.setReset(dummy_reset);
tran1.setLabels({MHyProLabel('tran1')});

l1.addTransition(tran1);

%-----------------------------------------------%
%              loc2 --> loc3
%-----------------------------------------------%
tran2 = MHyProTransition();
% Set guard: x1 = 2 
guard2 = MHyProCondition();
guard2.setMatrix([1 0 0 0 0; -1 0 0 0 0]); 
guard2.setVector([2;-2]);

tran2.setAggregation(1);
tran2.setGuard(guard2);
tran2.setSource(l2);
tran2.setTarget(l3);
tran2.setReset(dummy_reset);
tran2.setLabels({MHyProLabel('tran2')});

l2.addTransition(tran2);

%-----------------------------------------------%
%              loc3 --> loc4
%-----------------------------------------------%
tran3 = MHyProTransition();
% Set guard: x1 = 1 
guard3 = MHyProCondition();
guard3.setMatrix([1 0 0 0 0; -1 0 0 0 0]); 
guard3.setVector([1;-1]);

tran3.setAggregation(1);
tran3.setGuard(guard3);
tran3.setSource(l3);
tran3.setTarget(l4);
tran3.setReset(dummy_reset);
tran3.setLabels({MHyProLabel('tran3')});

l3.addTransition(tran3);

%-----------------------------------------------%
%              loc4 --> loc5
%-----------------------------------------------%
tran4 = MHyProTransition();
% Set guard: x1 = 0 
guard4 = MHyProCondition();
guard4.setMatrix([1 0 0 0 0; -1 0 0 0 0]); 
guard4.setVector([0;0]);

tran4.setAggregation(1);
tran4.setGuard(guard4);
tran4.setSource(l4);
tran4.setTarget(l5);
tran4.setReset(dummy_reset);
tran4.setLabels({MHyProLabel('tran4')});

l4.addTransition(tran4);


%-----------------------------------------------%
%                 Initial set
%-----------------------------------------------%

% x1 = [3.1, 3.1] x2 = [4,4] x3 = [0,0] x4 = [0,0] x5 = [0,0]
boxVector = [3.1;4;0;0;0;-3.1;-4;0;0;0];
boxMatrix = [eye(5);-1*eye(5)];
initialCond = MHyProCondition(boxMatrix, boxVector);
automaton.addInitialState(l1, initialCond);

%-----------------------------------------------%
%                 Reachability
%-----------------------------------------------%

settings = struct('timeStep', 0.001, 'timeBound', 2, 'jumpDepth', 5);
reach = MHyProReach(automaton);
reach.setSettings(settings);
reach.setRepresentationType(0);
%reach.settings();

tic;
flowpipes = reach.computeForwardReachability();
time = toc;
disp(['Time needed: ', num2str(time)]);
dim = [1 2];
reach.plot(flowpipes, dim);

end