function ioWriteGEOMfac(varargin)

% FUNCTION ioWriteGEOMfac(filename,geomdata,['noprompt'])
%
% DESCRIPTION
% This function writes a .geom file
%
% INPUT
%  filename       the filename for saving the data (append extension .geom)
%  geomdata       indices to the GEOM structure or direct structured data
% 
% OPTIONS
%  noprompt       do not prompt before overwriting a file
%
% OUTPUT
%  -
%
% GEOM STRUCTURE
%  .filename      - filename
%  .name          - the name of the data (where it came from)
%  .pts           - pts file
%  .cpts          - the conductivity tensors per point
%  .fac           - fac file
%  .cfac          - the conductivity tensors per element
%  .channels      - the channels file
%
% SEE ALSO ioReadGEOM, ioReadGEOMdata, ioWriteGEOM, ioWriteGEOMdata

geomfiles = {};
geomdata = {};
noprompt = 0;

for p=1:nargin,
 
   fparam = varargin{p};

   if ischar(fparam)
       switch fparam
       case 'noprompt'
          noprompt = 1;
       otherwise
          geomfiles{end+1} = fparam;
       end
   end

   if isstruct(fparam), 
      geomdata{end+1} = fparam;
   end

   if isnumeric(fparam),
       global GEOM;
       for q=1:length(fparam),
            geomdata{end+1} = GEOM{fparam(q)};
	end
   end

   if iscell(fparam),
      for q=1:length(fparam),
          if isstruct(fparam{q}),
              geomdata{end+1} = fparam{q};
          end
          if ischar(fparam{q}),
              geomfiles{end+1} = fparam{q};
          end
      end 
   end
end


if (length(geomfiles) == 1) & (length(geomdata) > 1),
     
    msgError('Only one geometry per file is allowed',5);
    return; 
        
elseif (length(geomfiles)==length(geomdata)),

    for p =1:length(geomfiles),
        [pn,fn,ext] = fileparts(geomfiles{p});
        geomfiles{p} = fullfile(pn,fn);
    
    if ~isempty(geomdata{p}.fac),
        result = 1;
        if exist([geomfiles{p} '.fac'],'file')    
            if noprompt ~= 1,			% be careful altering these statements as they allow for overwriting files
                question = sprintf('Do you want to replace: %s ?',[geomfiles{p} '.fac']);
                result = utilQuestionYN(question);
            end
        end
        if result==1,
            ioWriteFac([geomfiles{p} '.fac'],geomdata{p}.fac);
        end
    end
        
    if ~isempty(geomdata{p}.pts),
        result = 1;
        if exist([geomfiles{p} '.pts'],'file')    
            if noprompt ~= 1,			% be careful altering these statements as they allow for overwriting files
                question = sprintf('Do you want to replace: %s ?',[geomfiles{p} '.pts']);
                result = utilQuestionYN(question);
            end
        end
        if result==1,
            ioWritePts([geomfiles{p} '.pts'],geomdata{p}.pts);
        end
    end    
    
    if isfield(geomdata{p},'channels'),
    if ~isempty(geomdata{p}.channels),
        result = 1;
        if exist([geomfiles{p} '.channels'],'file')    
            if noprompt ~= 1,			% be careful altering these statements as they allow for overwriting files
                question = sprintf('Do you want to replace: %s ?',[geomfiles{p} '.channels']);
                result = utilQuestionYN(question);
            end
        end
        if result==1,
            ioWriteChannels([geomfiles{p} '.channels'],geomdata{p}.channels);
        end
    end    
    end
    end

else
    msgError('The number of geometries should match the number of filenames',5);
    return;
end
