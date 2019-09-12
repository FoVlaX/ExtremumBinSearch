float4 VS( float4 Pos : POSITION ) : SV_POSITION
{
	//оставляем все без изменений
	return Pos;
}

float4 PS(float4 Pos : SV_POSITION ) : SV_Target
{
	float flimiter = 800.0f;
	float dist = Pos.y*Pos.x + Pos.y*Pos.x;
	dist = (dist%flimiter)/flimiter;
	return float4( 0.7*dist, 0.5*dist, 0.8*dist, 1.0f );
}