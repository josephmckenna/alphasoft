#include"VertexFCN.hh"
#include<cassert>

Hel2VtxFCN::Hel2VtxFCN(TFitVertex* a_vtx) : vtx(*a_vtx), error_def(1) {
	hellColl = vtx.GetHelixStack();
}

double Hel2VtxFCN::Up() const {
	return error_def;
}

VertFuncFCN::VertFuncFCN(TFitVertex* a_vtx) : Hel2VtxFCN(a_vtx) {
}

double VertFuncFCN::operator()(const std::vector<double>& p) const {
	//assert(p.size() == 5);

	double chi2 = 0;
    double tx,ty,tz,s;
    const int helpoints = hellColl->size();
    for(int i=0; i<helpoints; ++i)
    {
      s=p[i+3];
      const TVector3& h = hellColl->at(i)->GetPosition(s);
      //      TVector3 e2 = ( (TFitHelix*) hellColl->At(i) )->EvaluateErrors2(h.Perp2());
      const TVector3& e2 = hellColl->at(i)->GetError2(s);
      tx=p[0]-h.X(); ty=p[1]-h.Y(); tz=p[2]-h.Z();
      chi2 += tx*tx/e2.X() + ty*ty/e2.Y() + tz*tz/e2.Z() ;
    }
	return chi2;
}



MinDistFunctionFCN::MinDistFunctionFCN(TFitVertex* a_vtx) : vtx(*a_vtx), error_def(1) {
	hel0 = vtx.GetInit0();
    hel1 = vtx.GetInit1();
}

double MinDistFunctionFCN::Up() const {
	return error_def;
}

MinDistFCN::MinDistFCN(TFitVertex* a_vtx) : MinDistFunctionFCN(a_vtx) {
}

double MinDistFCN::operator()(const std::vector<double>& p) const {
	//assert(p.size() == 5);

	double chi2 = 0;
    TVector3 H0     = hel0->GetPosition(p[0]);
    TVector3 H0err2 = hel0->GetError2(p[0]);
    TVector3 H1     = hel1->GetPosition(p[1]);
    TVector3 H1err2 = hel1->GetError2(p[1]);
	double tx=H0.X()-H1.X(), ty=H0.Y()-H1.Y(), tz=H0.Z()-H1.Z();
    chi2 = tx*tx/(H0err2.X()+H1err2.X())
         + ty*ty/(H0err2.Y()+H1err2.Y())
         + tz*tz/(H0err2.Z()+H1err2.Z());
    
    return chi2;
}

