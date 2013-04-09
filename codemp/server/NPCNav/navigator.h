#ifndef __G_NAVIGATOR__
#define __G_NAVIGATOR__

#define	__NEWCOLLECT	1

#define _HARD_CONNECT	1

//Node flags
#define	NF_ANY			0
//#define	NF_CLEAR_LOS	0x00000001
#define NF_CLEAR_PATH	0x00000002
#define NF_RECALC		0x00000004

//Edge flags
#define	EFLAG_NONE		0
#define EFLAG_BLOCKED	0x00000001
#define EFLAG_FAILED	0x00000002

//Miscellaneous defines
#define	NODE_NONE		-1
#define	NAV_HEADER_ID	'JNV5'
#define	NODE_HEADER_ID	'NODE'

#pragma warning( disable : 4786) 

#if defined(_WIN32) && !defined(_XBOX)
#define COM_NO_WINDOWS_H
#include <objbase.h>
#endif

#include <map>
#include <vector>
#include <list>
using namespace std;

#include "../server.h"
#include "../../game/q_shared.h"


typedef multimap<int, int> EdgeMultimap;
typedef multimap<int, int>::iterator EdgeMultimapIt;


/*
-------------------------
CEdge
-------------------------
*/

class CEdge
{

public:

	CEdge( void );
	CEdge( int first, int second, int cost );
	~CEdge( void );

	int		m_first;
	int		m_second;
	int		m_cost;
};

/*
-------------------------
CNode
-------------------------
*/

class CNode
{
	typedef	struct edge_s
	{
		int		ID;
		int		cost;
		BYTE	flags;
	} edge_t;

	typedef	vector< edge_t >	edge_v;

public:

	CNode( void );
	~CNode( void );

	static CNode *Create( vec3_t position, int flags, int radius, int ID );
	static CNode *Create( void );

	void AddEdge( int ID, int cost, int flags = EFLAG_NONE );
	void AddRank( int ID, int rank );

	void Draw( qboolean radius );

	int	GetID( void )					const	{	return m_ID;	}
	void GetPosition( vec3_t position )	const	{	if ( position )	VectorCopy( m_position, position );	}
 	
	int GetNumEdges( void )				const	{	return m_numEdges;	}
	int	GetEdgeNumToNode( int ID );
	int GetEdge( int edgeNum );
	int GetEdgeCost( int edgeNum );
	BYTE GetEdgeFlags( int edgeNum );
	void SetEdgeFlags( int edgeNum, int newFlags );
	int	GetRadius( void )				const	{	return m_radius;	}

	void InitRanks( int size );
	int GetRank( int ID );

	int	GetFlags( void )				const	{	return m_flags;	}
	void AddFlag( int newFlag )			{	m_flags |= newFlag;	}
	void RemoveFlag( int oldFlag )		{	m_flags &= ~oldFlag; }

	int	Save( int numNodes, fileHandle_t file );
	int Load( int numNodes, fileHandle_t file );

protected:

	vec3_t			m_position;
	int				m_flags;
	int				m_radius;
	int				m_ID;
	
	edge_v	m_edges;

	int		*m_ranks;
	int		m_numEdges;
};

/*
-------------------------
CNavigator
-------------------------
*/
#define MAX_FAILED_EDGES	32
class CNavigator
{
	typedef	vector < CNode * >			node_v;
	typedef	list < CEdge >				edge_l;

#if __NEWCOLLECT
	
	typedef struct nodeList_t
	{
		int				nodeID;
		unsigned int	distance;
	};

	typedef list < nodeList_t >		nodeChain_l;

#endif	//__NEWCOLLECT

public:

	CNavigator( void );
	~CNavigator( void );

	void Init( void );
	void Free( void );

	bool Load( const char *filename, int checksum );
	bool Save( const char *filename, int checksum );

	int  AddRawPoint( vec3_t point, int flags, int radius );
	void CalculatePaths( qboolean recalc=qfalse );

#if _HARD_CONNECT

	void HardConnect( int first, int second );

#endif

	void ShowNodes( void );
	void ShowEdges( void );
	void ShowPath( int start, int end );

	int GetNearestNode( sharedEntity_t *ent, int lastID, int flags, int targetID );

	int GetBestNode( int startID, int endID, int rejectID = NODE_NONE );

	int GetNodePosition( int nodeID, vec3_t out );
	int GetNodeNumEdges( int nodeID );
	int GetNodeEdge( int nodeID, int edge );
	float GetNodeLeadDistance( int nodeID );

	int GetNumNodes( void )		const	{	return m_nodes.size();		}
	
	bool Connected( int startID, int endID );

	unsigned int GetPathCost( int startID, int endID );
	unsigned int GetEdgeCost( int startID, int endID );

	int GetProjectedNode( vec3_t origin, int nodeID );
//MCG Added BEGIN
	void CheckFailedNodes( sharedEntity_t *ent );
	void AddFailedNode( sharedEntity_t *ent, int nodeID );
	qboolean NodeFailed( sharedEntity_t *ent, int nodeID );
	qboolean NodesAreNeighbors( int startID, int endID );
	void ClearFailedEdge( failedEdge_t	*failedEdge );
	void ClearAllFailedEdges( void );
	int EdgeFailed( int startID, int endID );
	void AddFailedEdge( int entID, int startID, int endID );
	qboolean CheckFailedEdge( failedEdge_t *failedEdge );
	void CheckAllFailedEdges( void );
	qboolean RouteBlocked( int startID, int testEdgeID, int endID, int rejectRank );
	int GetBestNodeAltRoute( int startID, int endID, int *pathCost, int rejectID = NODE_NONE );
	int GetBestNodeAltRoute( int startID, int endID, int rejectID = NODE_NONE );
	int GetBestPathBetweenEnts( sharedEntity_t *ent, sharedEntity_t *goal, int flags );
	int	GetNodeRadius( int nodeID );
	void CheckBlockedEdges( void );
	void ClearCheckedNodes( void );
	byte CheckedNode(int wayPoint,int ent);
	void SetCheckedNode(int wayPoint,int ent,byte value);

	void FlagAllNodes( int newFlag );

	qboolean pathsCalculated;
//MCG Added END

protected:

	int		TestNodePath( sharedEntity_t *ent, int okToHitEntNum, vec3_t position, qboolean includeEnts );
	int		TestNodeLOS( sharedEntity_t *ent, vec3_t position );
	int		TestBestFirst( sharedEntity_t *ent, int lastID, int flags );
	
#if __NEWCOLLECT
	int		CollectNearestNodes( vec3_t origin, int radius, int maxCollect, nodeChain_l &nodeChain );
#else
	int		CollectNearestNodes( vec3_t origin, int radius, int maxCollect, int *nodeChain );
#endif	//__NEWCOLLECT

	char	GetChar( fileHandle_t file );
	int		GetInt( fileHandle_t file );
	float	GetFloat( fileHandle_t file );
	long	GetLong( fileHandle_t file );

	//void	ConnectNodes( void );
	void	SetEdgeCost( int ID1, int ID2, int cost );
	int		GetEdgeCost( CNode *first, CNode *second );
	void	AddNodeEdges( CNode *node, int addDist, edge_l &edgeList, bool *checkedNodes );

	void	CalculatePath( CNode *node );

	//rww - made failedEdges private as it doesn't seem to need to be public.
	//And I'd rather shoot myself than have to devise a way of setting/accessing this
	//array via trap calls.
	failedEdge_t	failedEdges[MAX_FAILED_EDGES];	

	node_v			m_nodes;
	EdgeMultimap	m_edgeLookupMap;
};

//////////////////////////////////////////////////////////////////////
// class Priority Queue
//////////////////////////////////////////////////////////////////////
class CPriorityQueue
{
// CONSTRUCTION /DESTRUCTION
//--------------------------------------------------------------
public:
	CPriorityQueue() {};
	~CPriorityQueue();
	
// Functionality
//--------------------------------------------------------------
public:
	CEdge*	Pop();
	CEdge*	Find(int npNum);
	void	Push( CEdge* theEdge );
	void	Update(CEdge* edge );
	bool	Empty();
	

// DATA
//--------------------------------------------------------------
private:
	vector<CEdge*>	mHeap;
};

extern CNavigator navigator;

#endif	//__G_NAVIGATOR__