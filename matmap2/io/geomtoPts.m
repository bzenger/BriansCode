geom = ioReadGEOMdata('/home/sci/kedar/Utah/svn/experiments/rsm-08-07-15/geom/needles08-07-15.geom');
pts = [];
% for i=1:10, for j=1:18, pts = [pts; geom{i}.pts(:)']; end; end
  for i=1:10 
      pts = [pts; geom{i}.pts(:,:)'];
  end
save /home/sci/kedar/Utah/svn/experiments/rsm-08-07-15/geom/needles08-07-15.pts -ascii pts