% testing affine fit 

x = [161.2474,268.9572,-56.2933
161.3458,268.9047,-56.2894
162.3061,268.2274,-56.3883
163.0436,268.5323,-56.2294
164.1372,268.3314,-56.0894
163.9304,270.4926,-56.2476
163.9304,270.4926,-56.2476
162.8756,271.0253,-56.0793
161.7697,271.4538,-56.2850
161.3713,271.4843,-56.4742
160.4215,269.8266,-56.7192
160.4215,269.8266,-56.7192
161.2239,268.9406,-56.3269
162.5874,267.4819,-56.6829
164.4860,268.2368,-56.4940
164.0119,269.4207,-56.1792
163.1427,270.4213,-56.2418
163.1427,270.4213,-56.2418
161.0679,270.7860,-56.2531
162.1061,269.4072,-56.4431
163.6223,269.2353,-56.5580
162.2191,270.6411,-56.5292
162.2191,270.6411,-56.5292
159.7404,268.4180,-56.5664
163.5878,267.9836,-56.4487
164.3740,270.7586,-56.2783
160.7530,270.9756,-55.9915
160.6288,269.5837,-56.3261
160.6288,269.5837,-56.3261
162.7150,268.5583,-55.9007
162.0139,270.2368,-55.7540
161.7367,269.3525,-55.9422
161.5387,269.8868,-55.8087];

% normalize x 
maxX = max(abs(x)); 

normalx = [x(:,1)./maxX(1) , x(:,2)./maxX(2) , x(:,3)./maxX(3)]; 

%test affine thing

[n,p,v] = affine_fit(x)

%plot
scatter3(normalx(:,1),normalx(:,2),normalx(:,3))
hold on
plot3([0,n(1)],[0,n(2)],[0,n(3)])
xlim([-1,1])
ylim([-1,1])
zlim([-1,1])
scatter3(0,0,0)