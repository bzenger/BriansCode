%demo for affine_fit
%Author: Adrien Leygue
%Date: December 3 2014
close all
clear all
figure;
%generate points that lie approximately in the Z=0 plane
N = 10;
%[X,Y] = meshgrid(linspace(0,1,N));
%XYZ_1 = [X(:) Y(:) 0.05*randn(N^2,1)];
XYZ_1 = [161.2474,268.9572,-56.2933
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
160.6288,269.5837,-56.3261
160.6288,269.5837,-56.3261];

plot3(XYZ_1(:,1),XYZ_1(:,2),XYZ_1(:,3),'r.');
hold on
%compute the normal to the plane and a point that belongs to the plane
[n_1,V_1,p_1] = affine_fit(XYZ_1);

%generate points that lie approximately in the Z=X plane
%the normal vector is


%plot the two points p_1 and p_2
plot3(p_1(1),p_1(2),p_1(3),'ro','markersize',15,'markerfacecolor','red');

%plot the normal vector
quiver3(p_1(1),p_1(2),p_1(3),n_1(1)/3,n_1(2)/3,n_1(3)/3,'r','linewidth',2)
% h = quiver3(p_2(1),p_2(2),p_2(3),n_2(1)/3,n_2(2)/3,n_2(3)/3,'b','linewidth',2)

[S1,S2] = meshgrid([-3 0 3]);
%generate the pont coordinates
X = p_1(1)+[S1(:) S2(:)]*V_1(1,:)';
Y = p_1(2)+[S1(:) S2(:)]*V_1(2,:)';
Z = p_1(3)+[S1(:) S2(:)]*V_1(3,:)';
%plot the plane
surf(reshape(X,3,3),reshape(Y,3,3),reshape(Z,3,3),'facecolor','blue','facealpha',0.5);

%plot the two adjusted planes
%[X,Y] = meshgrid(linspace(0,1,3));

%first plane
%surf(X,Y, - (n_1(1)/n_1(3)*X+n_1(2)/n_1(3)*Y-dot(n_1,p_1)/n_1(3)),'facecolor','red','facealpha',0.5);

%second plane
%NB: if the plane is vertical the above method cannot be used, one should
%use the secont output of affine_fit which contains a base of the plane.
%this is illustrated below
%S1 and S2 are the coordinates of the plane points in the basis made of the
%columns ov V_2
%[S1,S2] = meshgrid([-1 0 1]);
%generate the pont coordinates
%X = p_2(1)+[S1(:) S2(:)]*V_2(1,:)';
%Y = p_2(2)+[S1(:) S2(:)]*V_2(2,:)';
%Z = p_2(3)+[S1(:) S2(:)]*V_2(3,:)';
%plot the plane
%surf(reshape(X,3,3),reshape(Y,3,3),reshape(Z,3,3),'facecolor','blue','facealpha',0.5);

xlabel('x');
ylabel('y');
zlabel('z');
axis equal
%compute the angle between the planes in [0 90] degrees
% angle = acosd(dot(n_1,n_2));
% if angle>90
%     angle = 180-angle;
% end
% angle
