function TSmapindex = sigActRecMap(TSindex,refmode)

    TSmapindex = [];

    if isnumeric(TSindex)
        global TS;
  
        for p=TSindex,
            act = fidsFindLocalFids(p,'act');
            if ~isempty(act),
                act = act(:,1); % only get the first series
            end
            
            rec = fidsFindLocalFids(p,'rec');
            if ~isempty(rec),
                rec = rec(:,1); % only get the first series
            end
          
            ref = 0;
            rf = fidsFindGlobalFids(p,'reference');
            if (~isempty(rf)),
                ref = (rf(1)-1);
            end
            
            act = act - ref;
            rec = rec - ref;
            
            if length(act) == length(rec), 
                ari = rec-act; 
            else
                ari = [];    
            end
            
            numchannels = size(TS{p}.potvals,1);
            datamap = zeros(numchannels,3);
            if ~isempty(act), if length(act) == numchannels, datamap(:,1) = act; end, end
            if ~isempty(rec), if length(rec) == numchannels, datamap(:,2) = rec; end, end
            if ~isempty(ari), if length(ari) == numchannels, datamap(:,3) = ari; end, end
    
            q = tsNew(1);
            TS{q} = TS{p};
            TS{q}.potvals = datamap;
            TS{q}.numleads = numchannels;
            TS{q}.numframes = 3;
            TS{q}.pacing  = [];
            TS{q}.fids = [];
            TS{q}.fidset = {};
            TS{q}.audit = [TS{q}.audit '| Activation/Recovery/ARInterval map'];
    
            TSmapindex = [TSmapindex q];
        end
    end
    
    return