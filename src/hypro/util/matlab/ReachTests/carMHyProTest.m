function carMHyProTest

% Create Automaton
automaton = MHyProHAutomaton();

%-----------------------------------------------%
%              Location acc
%-----------------------------------------------%
loc_acc = MHyProLocation();
loc_acc.setName('acc');

% Set flow: v' = 5 d' = 19.44 - v t' = 1
dynAcc = [0 0 0 5; -1 0 0 19.44; 0 0 0 1; 0 0 0 0];
loc_acc.setFlow(dynAcc);

% Set inv: d >= 20
invAcc = MHyProCondition([0 -1 0], -20);
loc_acc.setInvariant(invAcc);


%-----------------------------------------------%
%              Location brake
%-----------------------------------------------%
loc_brake = MHyProLocation();
loc_brake.setName('brake');

% Set flow: v' = -7 d' = 19.44 - v t' = 1
dynBrake = [0 0 0 -7; -1 0 0 19.44; 0 0 0 1; 0 0 0 0];
loc_brake.setFlow(dynBrake);

% Set inv: v >= 0
invBrake = MHyProCondition([-1 0 0], 0);
loc_brake.setInvariant(invBrake);


%-----------------------------------------------%
%              Location idle
%-----------------------------------------------%
loc_idle = MHyProLocation();
loc_idle.setName('idle');

% Set flow: v' = 0 d' = 19.44 - v t' = 1
dynIdle = [0 0 0 0; -1 0 0 19.44; 0 0 0 1; 0 0 0 0];
loc_idle.setFlow(dynIdle);

% Set inv: d >= 20
invIdle = MHyProCondition([0 -1 0], -20);
loc_brake.setInvariant(invIdle);

%-----------------------------------------------%
%              acc -> brake
%-----------------------------------------------%

tran1 = MHyProTransition();
tran1.setAggregation(0);
tran1.setSource(loc_acc);
tran1.setTarget(loc_brake);
tran1.setLabels({MHyProLabel('tran1')});
loc_acc.addTransition(tran1);

%-----------------------------------------------%
%              brake -> acc
%-----------------------------------------------%

tran2 = MHyProTransition();
% d >= 20
guard2 = MHyProCondition();
guard2.setMatrix([0 -1 0]);
guard2.setVector(-20);
tran2.setGuard(guard2);
tran2.setAggregation(0);
tran2.setSource(loc_brake);
tran2.setTarget(loc_acc);
tran2.setLabels({MHyProLabel('tran2')});
loc_brake.addTransition(tran2);

%-----------------------------------------------%
%              brake -> idle
%-----------------------------------------------%

tran3 = MHyProTransition();
tran3.setAggregation(0);
tran3.setSource(loc_brake);
tran3.setTarget(loc_idle);
tran3.setLabels({MHyProLabel('tran3')});
loc_brake.addTransition(tran3);

%-----------------------------------------------%
%              idle -> acc
%-----------------------------------------------%

tran4 = MHyProTransition();
% d >= 20
guard4 = MHyProCondition();
guard4.setMatrix([0 -1 0]);
guard4.setVector(-20);
tran4.setGuard(guard4);
tran4.setAggregation(0);
tran4.setSource(loc_idle);
tran4.setTarget(loc_acc);
tran4.setLabels({MHyProLabel('tran4')});
loc_idle.addTransition(tran4);

automaton.addLocation(loc_acc);
automaton.addLocation(loc_brake);
automaton.addLocation(loc_idle);

%-----------------------------------------------%
%                 Initial set
%-----------------------------------------------%

% v = 13.88 d = 50 t = 0
boxVector = [13.88; -13.88; 50; -50; 0; 0];
boxMatrix = [1 0 0; -1 0 0; 0 1 0; 0 -1 0; 0 0 1; 0 0 -1];
initialCond = MHyProCondition(boxMatrix, boxVector);
automaton.addInitialState(loc_acc, initialCond);

%-----------------------------------------------%
%                 Reachability
%-----------------------------------------------%

settings = struct('timeStep', 0.1, 'timeBound', 5, 'jumpDepth', 3);
reach = MHyProReach(automaton);
reach.setSettings(settings);
reach.setRepresentationType(0);
reach.settings();

tic;
flowpipes = reach.computeForwardReachability();
time = toc;
disp(['Time needed: ', num2str(time)]);
dim = [3 2];
reach.plot(flowpipes, dim);

end